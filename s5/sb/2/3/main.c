#define __AVR_ATmega328P__

#include <avr/io.h>
#include <util/delay.h>

#define BTN_RESET PB4
#define BTN_PREV PB0
#define BTN_NEXT PB1
#define BTN_PIN PINB
#define BTN_PORT PORTB

inline uint8_t BinaryToGray(uint8_t num)
{
    return num ^ (num >> 1);
}

int main() {
  UCSR0B &= ~_BV(RXEN0) & ~_BV(TXEN0);
  BTN_PORT |= _BV(BTN_RESET);
  BTN_PORT |= _BV(BTN_PREV);
  BTN_PORT |= _BV(BTN_NEXT);
  DDRD = 0xff;
  PORTD = 0x00;

  uint8_t i = 0, prev_i = 0;
  uint8_t held = 0;
  while (1) {
    if (!(PINB & _BV(BTN_PREV)) && !held) {
      i--;
      held = 1;
    }
    if (!(PINB & _BV(BTN_NEXT)) && !held) {
      i++;
      held = 1;
    }
    if (!(PINB & _BV(BTN_RESET))) {
      i = 0;
    }

    if ((PINB & _BV(BTN_PREV) && PINB & _BV(BTN_NEXT)) && held) {
      held = 0;
    }
    PORTD = BinaryToGray(i);
    _delay_ms(10);
  }
}
