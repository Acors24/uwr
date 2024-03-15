#define __AVR_ATmega328P__
#define __DELAY_BACKWARD_COMPATIBLE__

#include <avr/pgmspace.h>
#include <avr/io.h>
#include <util/delay.h>
#include <stdio.h>
#include <inttypes.h>

#define BAUD 9600                          // baudrate
#define UBRR_VALUE ((F_CPU)/16/(BAUD)-1)   // zgodnie ze wzorem

// inicjalizacja UART
void uart_init()
{
  // ustaw baudrate
  UBRR0 = UBRR_VALUE;
  // wyczyść rejestr UCSR0A
  UCSR0A = 0;
  // włącz odbiornik i nadajnik
  UCSR0B = _BV(RXEN0) | _BV(TXEN0);
  // ustaw format 8n1
  UCSR0C = _BV(UCSZ00) | _BV(UCSZ01);
}

// transmisja jednego znaku
int uart_transmit(char data, FILE *stream)
{
  // czekaj aż transmiter gotowy
  while(!(UCSR0A & _BV(UDRE0)));
  UDR0 = data;
  return 0;
}

// odczyt jednego znaku
int uart_receive(FILE *stream)
{
  // czekaj aż znak dostępny
  while (!(UCSR0A & _BV(RXC0)));
  return UDR0;
}

FILE uart_file;

// inicjalizacja ADC
void adc_init()
{
  ADMUX   = _BV(REFS0); // referencja AVcc, wejście ADC0
  ADMUX  |= 0x01;       // wejście ADC1!
  DIDR0   = _BV(ADC1D); // wyłącz wejście cyfrowe na ADC1
  // częstotliwość zegara ADC 125 kHz (16 MHz / 128)
  ADCSRA  = _BV(ADPS0) | _BV(ADPS1) | _BV(ADPS2); // preskaler 128
  ADCSRA |= _BV(ADEN);  // włącz ADC
  ADCSRA |= _BV(ADATE); // auto-trigger
  ADCSRA |= _BV(ADSC);  // start conversion
}

FILE uart_file;

volatile int main()
{
  // zainicjalizuj UART
  uart_init();
  // skonfiguruj strumienie wejścia/wyjścia
  fdev_setup_stream(&uart_file, uart_transmit, uart_receive, _FDEV_SETUP_RW);
  stdin = stdout = stderr = &uart_file;
  // zainicjalizuj ADC
  adc_init();

  uint32_t v, R;
  const uint32_t B = 1821;
  const uint32_t R0 = 4700, T0x100 = 29815;
  uint32_t T;

  while(1) {
    // while (!(ADCSRA & _BV(ADIF))); // czekaj na wynik
    if (ADCSRA & _BV(ADIF))
      ADCSRA |= _BV(ADIF); // wyczyść bit ADIF (pisząc 1!)
    v = ADC; // weź zmierzoną wartość (0..1023)
    R = 2200 * (1023.0 / v) - 2200;

    T = (B * T0x100 / 100.0) / (log(R / (float) R0) * T0x100 / 100.0 + B) - 273.15;
    
    printf("%d\r\n", T);

    _delay_ms(500);
  }
}
