#define __AVR_ATmega328P__

#include <avr/io.h>
#include <util/delay.h>

#define LED PB5
#define LED_DDR DDRB
#define LED_PORT PORTB

#define BTN PB4
#define BTN_PIN PINB
#define BTN_PORT PORTB

const uint8_t BUFFER_SIZE = 100;
uint8_t buffer[100];
uint8_t i = 0;

void adv() {
  if (buffer[i])
    LED_PORT &= ~_BV(LED);
  else
    LED_PORT |= _BV(LED);
  buffer[i] = BTN_PIN & _BV(BTN);
  i++;
  if (i == BUFFER_SIZE)
    i = 0;
}

int main() {
  BTN_PORT |= _BV(BTN);
  LED_DDR |= _BV(LED);
  while (1) {
    adv();
    _delay_ms(10);
  }
}
