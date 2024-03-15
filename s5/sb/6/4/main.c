// L6Z4

#define __AVR_ATmega328P__

#include <avr/io.h>
#include <util/delay.h>

inline void spi_init()
{
  // włącz, ustaw jako master, clock rate /128, inny data order
  SPCR  = _BV(SPE) | _BV(MSTR) | _BV(SPR1) | _BV(SPR0) | _BV(DORD);
  DDRB |= _BV(PB1); // ¬LA
  DDRB |= _BV(PB2); // SS / ¬OE
  DDRB |= _BV(PB3); // MOSI
  DDRB |= _BV(PB5); // SCK
}

inline void load(const uint8_t data)
{
  PORTB &= ~_BV(PB1);

  SPDR = data;
  while (!(SPSR & _BV(SPIF)));
  SPSR &= ~_BV(SPIF);

  PORTB |=  _BV(PB1);
}

const uint8_t digits[] = {
  0b11111100,
  0b01100000,
  0b11011010,
  0b11110010,
  0b01100110,
  0b10110110,
  0b10111110,
  0b11100000,
  0b11111110,
  0b11110110,
};

int main()
{
  spi_init();

  uint8_t i = 0;
  while (1)
  {
    load(digits[i]);
    _delay_ms(500);
    load(digits[i] | 1);
    _delay_ms(500);
    i *= ++i != 10;
  }
}
