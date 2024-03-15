// L10Z4

#include <avr/pgmspace.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>
#include <avr/io.h>
#include <util/delay.h>
#include "dzwiek.h"

#define CS PB2

void spi_init()
{
  // włącz, ustaw jako master, clock rate /128
  SPCR = _BV(SPE) | _BV(MSTR) | _BV(SPR1) | _BV(SPR1);
  // SPSR |= _BV(SPI2X);
  PORTB |= _BV(CS);
  DDRB  |= _BV(CS);
  DDRB  |= _BV(PB3); // MOSI
  DDRB  |= _BV(PB5); // SCK
}

static inline void load(uint8_t data)
{
  PORTB &= ~_BV(CS);

  SPDR = 0b00110000 | (data >> 4);
  while (!(SPSR & _BV(SPIF)));
  SPSR &= ~_BV(SPIF);

  SPDR = data << 4;
  while (!(SPSR & _BV(SPIF)));
  SPSR &= ~_BV(SPIF);

  PORTB |= _BV(CS);
}

int main()
{
  spi_init();

  uint64_t i = 0;
  uint8_t data;

  while (1)
  {
    data = pgm_read_byte(dzwiek_raw + i++);
    load(data);
    // _delay_us(DELAY);
    // i *= i < dzwiek_raw_len;
    if (i == dzwiek_raw_len) {
      i = 0;
      _delay_ms(500);
    }
  }
}
