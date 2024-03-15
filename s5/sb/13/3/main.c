// L13Z3

#include <avr/interrupt.h>
#include <avr/sleep.h>
#include <avr/io.h>
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
  ADMUX = _BV(REFS1) | _BV(REFS0);
  // częstotliwość zegara ADC 125 kHz (16 MHz / 128)
  ADCSRA = _BV(ADPS2) | _BV(ADPS1) | _BV(ADPS0); // preskaler 128
  // ADCSRA = _BV(ADPS1); // preskaler 8
  ADCSRA |= _BV(ADEN);
  ADCSRA |= _BV(ADIE); // interrupt enable
  DIDR0  = _BV(ADC0D); // wyłącz wejście cyfrowe na ADC0
  set_sleep_mode(SLEEP_MODE_IDLE);
}

volatile uint16_t adc_v;
ISR(ADC_vect) {
  adc_v = ADC;
}

int main() {
  // zainicjalizuj UART
  uart_init();
  // skonfiguruj strumienie wejścia/wyjścia
  fdev_setup_stream(&uart_file, uart_transmit, uart_receive, _FDEV_SETUP_RW);
  stdin = stdout = stderr = &uart_file;
  // program testowy
  adc_init();
  sei();

  uint32_t R, v_in_mV;
  // const uint32_t B = 1821;
  const uint32_t B = 4000;
  // const uint32_t B = 3000;
  const uint32_t R0 = 4700;
  const float T0 = 298.15;
  uint32_t T;

  while (1) {
    ADCSRA |= _BV(ADSC);
    sleep_mode();

    v_in_mV = adc_v / 1024.0 * 1100;

    R = v_in_mV / 0.13;
    T = ((B * T0) / (log(R / (float) R0) * T0 + B) - 273.15) * 10;

    printf("%3lu.%1lu\r\n", T / 10, T % 10);
    _delay_ms(50);
  }
}
