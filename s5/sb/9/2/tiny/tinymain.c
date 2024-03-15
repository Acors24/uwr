// L9Z2

#include <avr/io.h>
#include <util/delay.h>

void spi_init() {
  // ustaw piny SCK I MOSI jako wyjścia
  DDRA = _BV(DDA4) | _BV(DDA5);
  // ustaw USI w trybie trzyprzewodowym (SPI)
  USICR = _BV(USIWM0);
}

uint8_t spi_transfer(uint8_t data) {
  // załaduj dane do przesłania
  USIDR = data;
  // wyczyść flagę przerwania USI
  USISR = _BV(USIOIF);
  // póki transmisja nie została ukończona, wysyłaj impulsy zegara
  while (!(USISR & _BV(USIOIF))) {
    // wygeneruj pojedyncze zbocze zegarowe
    // zostanie wykonane 16 razy
    USICR = _BV(USIWM0) | _BV(USICS1) | _BV(USICLK) | _BV(USITC);
  }
  // zwróć otrzymane dane
  return USIDR;
}

int main() {
  DDRB |= _BV(PB2);  // LED
  PORTA |= _BV(PA7); // przycisk
  spi_init();
  uint8_t slave_data;
  while (1) {
    slave_data = spi_transfer(!(PINA & _BV(PA7)));
    if (slave_data) {
      PORTB |= _BV(PB2);
    } else {
      PORTB &= ~_BV(PB2);
    }
    // _delay_ms(10);
  }
}
