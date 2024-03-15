// L13Z4

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

void timer0_init() {
  TCCR0A = _BV(COM0A0) | _BV(WGM01); // CTC; toggle
  TCCR0B = _BV(CS02) | _BV(CS00);    // preskaler 1024

  // 16e6 / 2 / 1024 / 77 - 1 ≈ 100 Hz
  OCR0A = 77;
  // OCR0A = 128;
  // OCR0A = 250;

  DDRD |= _BV(PD6);
}

void timer1_init()
{
  // TCCR1B = _BV(CS10);
  TCCR1B = _BV(CS12) | _BV(CS10);
  // przerwanie przy zboczu narastającym na ICP1
  TIMSK1 = _BV(ICIE1);
}

uint8_t i = 0;
uint16_t v[2];

ISR(TIMER1_CAPT_vect)
{
  v[i] = ICR1;
  printf("%lu\r\n", 16000000 / 1024 / (v[i] - v[i ^ 1]));
  i ^= 1;
}

int main() {
  // zainicjalizuj UART
  uart_init();
  // skonfiguruj strumienie wejścia/wyjścia
  fdev_setup_stream(&uart_file, uart_transmit, uart_receive, _FDEV_SETUP_RW);
  stdin = stdout = stderr = &uart_file;
  // program testowy

  timer0_init();
  timer1_init();
  set_sleep_mode(SLEEP_MODE_IDLE);
  sei();

  while (1) {
    sleep_mode();
  }
}
