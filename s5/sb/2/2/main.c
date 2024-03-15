#define __AVR_ATmega328P__

#include <avr/io.h>
#include <util/delay.h>
#include <inttypes.h>
#include <stdio.h>

#define LED PB5
#define LED_DDR DDRB
#define LED_PORT PORTB

#define BTN PB4
#define BTN_PIN PINB
#define BTN_PORT PORTB

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


char morse[] = {
  ' ', 'E', 'T', 'I', 'A', 'N', 'M', 'S', 'U', 'R', 'W', 'D', 'K', 'G', 'O', 'H', 'V', 'F', 'Ü', 'L', 'Ä', 'P', 'J', 'B', 'X', 'C', 'Y', 'Z', 'Q'
};

const uint16_t DASH_THR = 500;
const uint16_t CHAR_THR = 1000;

int main() {
  // zainicjalizuj UART
  uart_init();
  // skonfiguruj strumienie wejścia/wyjścia
  fdev_setup_stream(&uart_file, uart_transmit, uart_receive, _FDEV_SETUP_RW);
  stdin = stdout = stderr = &uart_file;

  printf("ready\r\n");

  BTN_PORT |= _BV(BTN);
  LED_DDR |= _BV(LED);
  uint8_t was_held = 0, i = 1;
  uint32_t time_held = 0, time_waited = 0;
  while (1) {
    if (!(BTN_PIN & _BV(BTN))) { // przycisk jest naciśnięty
      time_held++;
      if (time_held >= DASH_THR) {
        LED_PORT |= _BV(LED);
      }
      was_held = 1;
      time_waited = 0;
    }
    else if (was_held) { // przycisk został puszczony
      i *= 2;
      if (time_held >= DASH_THR) {
        i += 1;
      } else {
      }
      was_held = 0;
      time_held = 0;
      LED_PORT &= ~_BV(LED);
    } else {
      if (time_waited++ >= CHAR_THR) {
        printf("%c", morse[i - 1]);
        i = 1;
        time_waited = 0;
      };
    }
    _delay_ms(1);
  }
}