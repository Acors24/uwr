// L10Z3

#include <avr/interrupt.h>
#include <avr/io.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
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
  ADCSRA |= _BV(ADATE) | _BV(ADSC) | _BV(ADEN);
  ADCSRA |= _BV(ADIE); // interrupt enable
  DIDR0 = _BV(ADC0D);  // wyłącz wejście cyfrowe na ADC0
}

// V_out = T_c * T_a + V_0
// ==>
// T_a = (V_out - V_0) / T_c
//
// V_0 = 500mV
// T_c = 10mV/C°
// ==>
// T_a = (V_out - 500mV) / 10mv/C°
volatile uint8_t temp;
volatile uint8_t T = 0;
ISR(ADC_vect) {
  temp = (110ull * ADC / 1023 - 50);
  if (temp > T) {
    DDRB &= ~_BV(PB5);
    PORTB &= ~_BV(PB5);
  } else if (temp < T - 1) {
    DDRB |= _BV(PB5);
    PORTB |= _BV(PB5);
  }
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

  char c;
  while (1) {
    c = getchar();
    if (c == 'r') {
      printf("temp: %u\r\n", temp);
    } else if (c == 'w') {
      scanf("%hhu", &T);
    }
  }
}
