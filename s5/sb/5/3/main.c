// L5Z3

#define __AVR_ATmega328P__

#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>
#include <stdio.h>
#include <inttypes.h>

#define BAUD 9600                            // baudrate
#define UBRR_VALUE ((F_CPU) / 16 / (BAUD)-1) // zgodnie ze wzorem

// inicjalizacja UART
void uart_init()
{
  // ustaw baudrate
  UBRR0 = UBRR_VALUE;
  // włącz odbiornik i nadajnik
  UCSR0B = _BV(RXEN0) | _BV(TXEN0);
  // ustaw format 8n1
  UCSR0C = _BV(UCSZ00) | _BV(UCSZ01);
}

// transmisja jednego znaku
int uart_transmit(char data, FILE *stream)
{
  // czekaj aż transmiter gotowy
  while (!(UCSR0A & _BV(UDRE0)))
    ;
  UDR0 = data;
  return 0;
}

// odczyt jednego znaku
int uart_receive(FILE *stream)
{
  // czekaj aż znak dostępny
  while (!(UCSR0A & _BV(RXC0)))
    ;
  return UDR0;
}

FILE uart_file;

void adc_init()
{
  ADMUX = _BV(REFS0) | _BV(REFS0) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1);
  // częstotliwość zegara ADC 125 kHz (16 MHz / 128)
  ADCSRA = _BV(ADPS2) | _BV(ADPS1) | _BV(ADPS0) | _BV(ADEN);
}

#define N 20
uint16_t sleep[N];
uint16_t normal[N];
uint8_t i;

ISR(ADC_vect)
{
  ADCSRA |= _BV(ADIF); // wyczyść bit ADIF (pisząc 1!)
  uint16_t v = ADC;    // weź zmierzoną wartość (0..1023)
  sleep[i] = v;
}

void var(float *_normal, float *_sleep)
{
  int32_t normalSum = 0, sleepSum = 0;
  for (uint8_t i = 0; i < N; i++)
  {
    normalSum += normal[i];
    sleepSum += sleep[i];
  }

  float normalAvg = normalSum / N;
  float sleepAvg = sleepSum / N;

  *_normal = 0;
  *_sleep = 0;
  for (uint8_t i = 0; i < N; i++)
  {
    *_normal += (normal[i] - normalAvg) * (normal[i] - normalAvg);
    *_sleep += (sleep[i] - sleepAvg) * (sleep[i] - sleepAvg);
  }

  *_normal /= N - 1;
  *_sleep /= N - 1;
}

int main()
{
  // zainicjalizuj UART
  uart_init();
  // skonfiguruj strumienie wejścia/wyjścia
  fdev_setup_stream(&uart_file, uart_transmit, uart_receive, _FDEV_SETUP_RW);
  stdin = stdout = stderr = &uart_file;
  // zainicjalizuj ADC
  adc_init();
  // odmaskuj przerwania
  sei();

  set_sleep_mode(SLEEP_MODE_ADC);
  // program testowy

  float normalVar, sleepVar;

  while (1)
  {
    for (i = 0; i < N; i++)
    {
      ADCSRA |= _BV(ADSC); // wykonaj konwersję
      while (!(ADCSRA & _BV(ADIF))); // czekaj na wynik
      ADCSRA |= _BV(ADIF); // wyczyść bit ADIF (pisząc 1!)
      uint16_t v = ADC;    // weź zmierzoną wartość (0..1023)
      normal[i] = v;
    }

    _delay_ms(200);

    ADCSRA |= _BV(ADIE);
    for (i = 0; i < N; i++)
    {
      sleep_mode();
    }
    ADCSRA &= ~_BV(ADIE);

    var(&normalVar, &sleepVar);

    printf("N * 100: %hu\r\n", (uint16_t)(normalVar * 100));
    printf("I * 100: %hu\r\n", (uint16_t)(sleepVar * 100));

    _delay_ms(1000);
  }
}
