// L6Z2

#define __AVR_ATmega328P__

#include <avr/interrupt.h>
#include <avr/sleep.h>
#include <avr/io.h>
#include <util/delay.h>
#include <stdio.h>

#define BAUD 9600                            // baudrate
#define UBRR_VALUE ((F_CPU) / 16 / (BAUD)-1) // zgodnie ze wzorem

// inicjalizacja UART
void uart_init()
{
  // ustaw baudrate
  UBRR0 = UBRR_VALUE;
  // wyczyść rejestr UCSR0A
  UCSR0A = 0;
  // włącz odbiornik, nadajnik oraz przerwania po odbiorze i opróżnieniu rejestru
  UCSR0B = _BV(RXEN0) | _BV(TXEN0) | _BV(RXCIE0) | _BV(UDRIE0);
  // ustaw format 8n1
  UCSR0C = _BV(UCSZ00) | _BV(UCSZ01);
}

volatile uint8_t tx_buffer[256];
volatile uint8_t tx_r = 0, tx_w = 0;
volatile uint8_t rx_buffer[256];
volatile uint8_t rx_r = 0, rx_w = 0;

// transmisja jednego znaku
int uart_transmit(char data, FILE *stream)
{
  // czekaj dopóki bufor nadawania pełny
  while (tx_w + 1 == tx_r)
    ;
  tx_buffer[tx_w++] = data;
  UCSR0B |= _BV(UDRIE0);
  return 0;
}

// odczyt jednego znaku
int uart_receive(FILE *stream)
{
  // czekaj dopóki bufor odbioru pusty
  while (rx_r == rx_w)
    ;
  return rx_buffer[rx_r++];
}

ISR(USART_RX_vect)
{
  uint8_t data = UDR0;
  // jeśli bufor odbioru niepełny
  if (rx_w + 1 != rx_r)
    rx_buffer[rx_w++] = data;
}

ISR(USART_UDRE_vect)
{
  // jeśli bufor nadawania niepusty
  if (tx_r != tx_w)
    UDR0 = tx_buffer[tx_r++];
  else
    UCSR0B &= ~_BV(UDRIE0);
}

FILE uart_file;

int main()
{
  // zainicjalizuj UART
  uart_init();
  // skonfiguruj strumienie wejścia/wyjścia
  fdev_setup_stream(&uart_file, uart_transmit, uart_receive, _FDEV_SETUP_RW);
  stdin = stdout = stderr = &uart_file;

  sei();

  // program testowy
  printf("Hello world!\r\n");
  char c;
  while (1)
  {
    scanf("%c", &c);
    printf("Odczytano: %c\r\n", c);
  }
}
