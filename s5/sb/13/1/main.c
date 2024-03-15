// L13Z1

#include <avr/interrupt.h>
#include <avr/sleep.h>
#include <avr/io.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <util/delay.h>

#define BAUD 9600                            // baudrate
#define UBRR_VALUE ((F_CPU) / 16 / (BAUD)-1) // zgodnie ze wzorem

// inicjalizacja UART
void uart_init() {
  // ustaw baudrate
  UBRR0 = UBRR_VALUE;
  // włącz odbiornik i nadajnik
  UCSR0B = _BV(RXEN0) | _BV(TXEN0);
  // ustaw format 8n1
  UCSR0C = _BV(UCSZ00) | _BV(UCSZ01);
}

// transmisja jednego znaku
int uart_transmit(char data, FILE *stream) {
  // czekaj aż transmiter gotowy
  while (!(UCSR0A & _BV(UDRE0)))
    ;
  UDR0 = data;
  return 0;
}

// odczyt jednego znaku
int uart_receive(FILE *stream) {
  // czekaj aż znak dostępny
  while (!(UCSR0A & _BV(RXC0)))
    ;
  return UDR0;
}

FILE uart_file;

void adc_init() {
  ADMUX = _BV(REFS0) | _BV(MUX0);
  // częstotliwość zegara ADC 125 kHz (16 MHz / 128)
  ADCSRA = _BV(ADPS2) | _BV(ADPS1) | _BV(ADPS0); // preskaler 128
  ADCSRA |= _BV(ADEN);
  ADCSRA |= _BV(ADIE); // interrupt enable
  DIDR0 = _BV(ADC1D);  // wyłącz wejście cyfrowe na ADC0
  set_sleep_mode(SLEEP_MODE_IDLE);
}

volatile uint16_t adc_v;
volatile uint8_t ok = 1;
ISR(ADC_vect) {
  adc_v = ADC;
  ok = 1;
}

void timer0_init() {
  TCCR0A = _BV(WGM01);
  TCCR0B = _BV(CS01); // preskaler 8

  // 16e6 / 2 / 8 / (1 + 124) ≈ 8 kHz
  OCR0A = 124;
  TIMSK0 |= _BV(OCIE0A);
}

ISR(TIMER0_COMPA_vect) {
  if (!ok)
    ADCSRA |= _BV(ADSC);
}

int main() {
  // zainicjalizuj UART
  uart_init();
  // skonfiguruj strumienie wejścia/wyjścia
  fdev_setup_stream(&uart_file, uart_transmit, uart_receive, _FDEV_SETUP_RW);
  stdin = stdout = stderr = &uart_file;
  // program testowy
  adc_init();
  timer0_init();
  sei();

  #define SAMPLE_SIZE_SQRT 4
  #define SAMPLE_SIZE (SAMPLE_SIZE_SQRT*SAMPLE_SIZE_SQRT)
  volatile uint16_t avg = 0;
  volatile uint16_t vs[SAMPLE_SIZE] = {0};
  volatile uint8_t i = 0;

  while (1) {
    if (ok) {
      avg -= vs[i];
      int16_t temp = (int16_t)(adc_v - 512) / SAMPLE_SIZE_SQRT;
      vs[i] = temp * temp;
      avg += vs[i++];
      i %= SAMPLE_SIZE;

      if (i == 0) {
        float x = sqrt(avg);
        printf("%7ld", (unsigned long)(20 * log10f(x / 2.5)));
      }
      ok = 0;
    }
  }
}
