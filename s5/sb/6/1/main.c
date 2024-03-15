// L6Z1

#include <stdint.h>
#define __AVR_ATmega328P__

#include <avr/interrupt.h>
#include <avr/sleep.h>
#include <avr/io.h>
#include <avr/delay.h>

#define BAUD 9600                            // baudrate
#define UBRR_VALUE ((F_CPU) / 16 / (BAUD)-1) // zgodnie ze wzorem

// inicjalizacja UART
void uart_init()
{
  // ustaw baudrate
  UBRR0 = UBRR_VALUE;
  // wyczyść rejestr UCSR0A
  UCSR0A = 0;
  // włącz odbiornik, nadajnik i przerwanie po odbiorze
  UCSR0B = _BV(RXEN0) | _BV(TXEN0) | _BV(RXCIE0);
  // ustaw format 8n1
  UCSR0C = _BV(UCSZ00) | _BV(UCSZ01);
}

ISR(USART_RX_vect)
{
  uint8_t in = UDR0;
  UDR0 = in;
  if (in == '\r')
    UDR0 = '\n';
}

int main()
{
  uart_init();
  sei();

  set_sleep_mode(SLEEP_MODE_IDLE);
  while (1) sleep_mode();
}
