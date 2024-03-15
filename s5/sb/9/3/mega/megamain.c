// L9Z3

#include "i2c.c"
#include "i2c.h"
#include <avr/interrupt.h>
#include <avr/io.h>
#include <stdint.h>
#include <stdio.h>
#include <util/delay.h>
#include <avr/sleep.h>

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

void i2c_init() {
  // ustaw adres tego slave'a
  const uint8_t slave_address = 0x7F;
  TWAR = (slave_address << 1); // | (TWAR & 0x1); // reakcja na general call
  // Enable Acknowledge, Enable
  TWCR |= _BV(TWEA) | _BV(TWEN);
  // ¬Start, ¬Stop
  TWCR &= ~_BV(TWSTA) & ~_BV(TWSTO);
}

int main() {
  // zainicjalizuj UART
  uart_init();
  // skonfiguruj strumienie wejścia/wyjścia
  fdev_setup_stream(&uart_file, uart_transmit, uart_receive, _FDEV_SETUP_RW);
  stdin = stdout = stderr = &uart_file;

  // zainicjalizuj I2C
  i2c_init();

  volatile uint8_t data[4];

  while (1) {
    data[0] = i2cReadAck(); // adres slave'a
    data[1] = i2cReadAck(); // adres pamięci?
    data[2] = i2cReadAck(); // dane
    data[3] = i2cReadAck(); // ?????
    printf("Otrzymano: %u\r\n", data[2]);
  }
}
