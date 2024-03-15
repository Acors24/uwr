// L4Z3

#define __AVR_ATmega328P__
#define __DELAY_BACKWARD_COMPATIBLE__

#include <avr/io.h>
#include <util/delay.h>
#include <stdio.h>
#include <inttypes.h>
#include <stdlib.h>
#include <time.h>

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

#include "wyniki.h"

#define LED_R PB1
#define LED_G PB2
#define LED_B PB3
#define LED_DDR DDRB
#define LED_PORT PORTB

void timer1_init()
{
  // ustaw tryb licznika
  // COM1A = 10   -- non-inverting mode
  // WGM1  = 1110 -- fast PWM top=ICR1
  // CS1   = 001  -- brak prescalera
  TCCR1A = _BV(COM1B1) | _BV(COM1A1) | _BV(WGM11);
  TCCR1B = _BV(WGM12) | _BV(WGM13) | _BV(CS10);
  ICR1 = 255;

  // COM2A = 10   -- non-inverting mode
  // WGM2  = 011  -- fast PWM top=0xFF
  // CS2   = 001  -- brak prescalera
  TCCR2A = _BV(COM2A1) | _BV(WGM21) | _BV(WGM20);
  TCCR2B = _BV(CS20);

  // ustaw piny OC1A:B i OC2A (PB1:3) jako wyjście
  LED_DDR |= _BV(LED_R) | _BV(LED_G) | _BV(LED_B);
}

void new_color(uint8_t colors[])
{
  uint8_t full = rand() % 3;
  uint8_t part;
  do {
    part = rand() % 3;
  } while (part == full);

  colors[0] = 255;
  colors[1] = 255;
  colors[2] = 255;

  colors[full] = 0;
  colors[part] = rand() % 256;
}

int main()
{
  srand(time(0));

  // zainicjalizuj UART
  uart_init();
  // skonfiguruj strumienie wejścia/wyjścia
  fdev_setup_stream(&uart_file, uart_transmit, uart_receive, _FDEV_SETUP_RW);
  stdin = stdout = stderr = &uart_file;

  // uruchom licznik
  timer1_init();

  uint8_t i = 0;
  uint8_t dir = 1;

  uint8_t r, g, b;
  int16_t v;
  uint8_t colors[3];

  while (1) {
    if (i == 0)
      new_color(colors);

    i += dir;
    v = func[i];

    r = (100 * v / 255 - 100) * colors[0] / 100 + 255;
    g = (100 * v / 255 - 100) * colors[1] / 100 + 255;
    b = (100 * v / 255 - 100) * colors[2] / 100 + 255;

    OCR1A = r;
    OCR1B = g;
    OCR2A = b;

    _delay_ms(3);

    dir *= 1 - 2 * (i == 0 || i == 255);
  }
}