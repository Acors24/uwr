// L4Z4

#define __AVR_ATmega328P__
#define __DELAY_BACKWARD_COMPATIBLE__

#include <avr/io.h>
#include <util/delay.h>
#include <stdio.h>
#include <inttypes.h>

#define BAUD 9600                            // baudrate
#define UBRR_VALUE ((F_CPU) / 16 / (BAUD)-1) // zgodnie ze wzorem

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

#define LED PB5
#define LED_DDR DDRB
#define LED_PORT PORTB

#define IR_REC PB0
#define IR_TRA PB1

void timer1_init()
{
  // ustaw tryb licznika
  // COM1A = 11   -- inverting mode
  // WGM1  = 1110 -- fast PWM top=ICR1
  // CS1   = 001  -- prescaler 1
  // 16e6 / (1 * (1 + 421)) ≈ 37.9 kHz
  ICR1 = 421;
  TCCR1A = _BV(COM1A1) | _BV(COM1A0) | _BV(WGM11);
  TCCR1B = _BV(WGM12) | _BV(WGM13) | _BV(CS10);

  // ustaw pin OC1A (PB1) jako wyjście
  DDRB |= _BV(IR_TRA);
  OCR1A = ICR1 / 2;

  PORTB |= _BV(IR_REC);
}

int main()
{
  // // zainicjalizuj UART
  // uart_init();
  // // // skonfiguruj strumienie wejścia/wyjścia
  // fdev_setup_stream(&uart_file, uart_transmit, uart_receive, _FDEV_SETUP_RW);
  // stdin = stdout = stderr = &uart_file;

  // uruchom licznik
  timer1_init();

  DDRB |= _BV(PB5);
  DDRB &= ~_BV(IR_TRA);
  while (1)
  {
    for (uint8_t i = 0; i < 5; i++)
    {
      DDRB |= _BV(IR_TRA);
      if (!(PINB & _BV(IR_REC)))
        PORTB |= _BV(PB5);
      else
        PORTB &= ~_BV(PB5);
      _delay_us(600);
      DDRB &= ~_BV(IR_TRA);
    }
    DDRB |= _BV(IR_TRA);
    if (!(PINB & _BV(IR_REC)))
      PORTB |= _BV(PB5);
    else
      PORTB &= ~_BV(PB5);
    _delay_us(600);
    DDRB &= ~_BV(IR_TRA);
    _delay_ms(93);
    _delay_us(400);
  }
}