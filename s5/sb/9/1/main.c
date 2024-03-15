// L9Z1

#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>

void io_init()
{
  // ustaw pull-up na PB0
  PORTA |= _BV(PA7);
  // ustaw wyjście na PB2
  DDRB |= _BV(PB2);
}

void timer0_init()
{
  // WGM 010 -- CTC top=OCRA
  // CS  101 -- prescaler 1024
  TCCR0A = _BV(WGM01);
  TCCR0B = _BV(CS01) | _BV(CS00);

  // 2e6 / (2 * 64 * (1 + 124)) = 128 Hz
  OCR0A = 124;

  // przerwanie przy porównaniu na kanale A
  TIMSK0 = _BV(OCIE0A);
}

// 16 * 8 = 128
volatile static uint8_t buffer[16];
volatile static uint8_t i = 0;

ISR(TIM0_COMPA_vect) {
  buffer[i >> 3] = (buffer[i >> 3] & ~(1 << (i & 7))) | (!!(PINA & _BV(PA7)) << (i & 7));
  i *= (++i != 128);
  PORTB = (PORTB & ~_BV(PB2)) | (_BV(PB2) * !((buffer[i >> 3]) & (1 << (i & 7))));
}

int main()
{
  // zainicjalizuj wejścia/wyjścia
  io_init();
  // zainicjalizuj licznik
  timer0_init();
  // odmaskuj przerwania
  sei();

  set_sleep_mode(SLEEP_MODE_IDLE);
  while(1) {
    sleep_mode();
  }
}