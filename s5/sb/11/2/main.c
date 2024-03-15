// L11Z2

#include <avr/interrupt.h>
#include <avr/io.h>
#include <avr/sleep.h>
#include <stdint.h>
#include <stdio.h>
#include <util/delay.h>

#define BAUD 9600                            // baudrate
#define UBRR_VALUE ((F_CPU) / 16 / (BAUD)-1) // zgodnie ze wzorem

// inicjalizacja UART
void uart_init() {
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
  ADMUX = _BV(REFS0);
  // częstotliwość zegara ADC 2 MHz (16 MHz / 8)
  ADCSRA = _BV(ADPS1) | _BV(ADPS1) | _BV(ADPS0); // preskaler 8
  ADCSRA |= _BV(ADSC) | _BV(ADEN);               // | _BV(ADIE);
  DIDR0 = _BV(ADC0D);  // wyłącz wejście cyfrowe na ADC0
  DIDR0 |= _BV(ADC1D); // wyłącz wejście cyfrowe na ADC1

  set_sleep_mode(SLEEP_MODE_IDLE);
}

volatile uint16_t adc_v = 0;
// ISR(ADC_vect) {
//   adc_v = ADC;
// }

uint16_t adsc(uint8_t mux) {
  ADMUX = (ADMUX & 0xF0) | (mux & 0x0F);
  ADCSRA |= _BV(ADSC);
  // sleep_mode();
  while (ADCSRA & _BV(ADIF))
    ;
  ADCSRA |= _BV(ADIF);
  adc_v = ADC;

  return adc_v;
}

void timer1_init() {
  // ustaw tryb licznika
  // COM1A = 10   -- clear on compare match
  // WGM1  = 1000 -- PFCPWM top=ICR1
  // CS1   = 011  -- /64
  TCCR1A = _BV(COM1A1);
  TCCR1B = _BV(WGM13) | _BV(CS11) | _BV(CS10);

  // f = 16e6 / 2 / N / TOP
  // f * TOP = 16e6 / 2 / N
  // TOP = 16e6 / 2 / N / f
  // 250 = 16e6 / 2 / 64 / 500
  ICR1 = 250;

  // ustaw pin OC1A (PB1) jako wyjście
  DDRB |= _BV(PB1);

  TIMSK1 = _BV(ICIE1) | _BV(TOIE1);
}

volatile uint16_t v1 = 0;
ISR(TIMER1_OVF_vect) {
  // sei();
  v1 = adsc(1);
  // cli();
}

volatile uint16_t v2 = 0;
ISR(TIMER1_CAPT_vect) {
  // sei();
  v2 = adsc(1);
  // cli();
}

int main() {
  // zainicjalizuj UART
  uart_init();
  // skonfiguruj strumienie wejścia/wyjścia
  fdev_setup_stream(&uart_file, uart_transmit, uart_receive, _FDEV_SETUP_RW);
  stdin = stdout = stderr = &uart_file;

  adc_init();
  timer1_init();

  sei();

  uint16_t i = 0;

  while (1) {
    OCR1A = (adsc(0) / 1023.0) * ICR1;
    if (i++ == 500) {
      // 5 - v1 / 1023.0 * 5
      // (5 - v1 / 1023.0 * 5) * 1000
      // (5000 - v1 * 4.887585532746823)
      printf("%4hhu | %4hhu\r", (uint16_t)(5000 - v1 * 4.887585532746823),
             (uint16_t)(v2 * 4.887585532746823));
      i = 0;
    }
  }
}
