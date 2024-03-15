// L9Z2

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#define SLAVE_MISO PB4

inline void spi_init() {
  // włącz SPI; włącz przerwanie po transferze;
  // // inny data order; inna faza i polaryzacja zegara
  SPCR = _BV(SPE) | _BV(SPIE);// | _BV(CPHA);// | _BV(CPOL);
  DDRB |= _BV(SLAVE_MISO);
}

volatile uint8_t master_data;
volatile uint8_t slave_data;
ISR(SPI_STC_vect) {
  master_data = SPDR;
  SPDR = slave_data;
}

int main()
{
    DDRD |= _BV(PD3);  // LED
    PORTD |= _BV(PD2); // przycisk

    spi_init();
    sei();

    while(1) {
        slave_data = !(PIND & _BV(PD2));
        if (master_data) {
            PORTD |= _BV(PD3);
        } else {
            PORTD &= ~_BV(PD3);
        }
    }
}

