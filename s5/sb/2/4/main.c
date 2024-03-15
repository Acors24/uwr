#define __AVR_ATmega328P__

#include <avr/io.h>
#include <util/delay.h>

#define TRN0_PORT PC0
#define TRN1_PORT PC1
#define TRN_DDR DDRC
#define TRN_PORT PORTC

#define LED_PORT PORTD
#define LED_DDR DDRD

const uint8_t wtf[] = {
  0b11000000, // 0
  0b11111001, // 1
  0b10100100, // 2
  0b10110000, // 3
  0b10011001, // 4
  0b10010010, // 5
  0b10000010, // 6
  0b11111000, // 7
  0b10000000, // 8
  0b10010000  // 9
};

int main() {
  TRN_DDR |= _BV(TRN0_PORT);
  TRN_DDR |= _BV(TRN1_PORT);

  TRN_PORT |= _BV(TRN0_PORT);
  TRN_PORT &= ~_BV(TRN1_PORT);

  LED_DDR = 0xff;
  uint16_t time = 0;
  uint8_t digit_t = 0, digit_o = 0;
  while (1) {
    time += 2;
    if (time == 1000) {
      digit_o++;
      if (digit_o == 10) {
        digit_t++;
        digit_o = 0;
        if (digit_t == 6) {
          digit_t = 0;
        }
      }
      time = 0;
    }

    LED_PORT = wtf[digit_t];
    TRN_PORT ^= _BV(TRN0_PORT);
    TRN_PORT ^= _BV(TRN1_PORT);
    _delay_ms(1);

    LED_PORT = wtf[digit_o];
    TRN_PORT ^= _BV(TRN0_PORT);
    TRN_PORT ^= _BV(TRN1_PORT);
    _delay_ms(1);
  }
}
