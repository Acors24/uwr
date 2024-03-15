// L5Z1

#define __AVR_ATmega328P__

#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>

void io_init()
{
  // ustaw pull-up na PD2 i PD3 (INT0 i INT1)
  PORTD |= _BV(PORTD0);
  // ustaw wyjście na PB5
  DDRB |= _BV(DDB5);
}

void timer0_init()
{
  // WGM 010 -- CTC top=OCRA
  // CS  101 -- prescaler 1024
  TCCR0A = _BV(WGM01);
  TCCR0B = _BV(CS02) | _BV(CS00);

  // 16e6 / (2 * 1024 * (1 + 60)) ≈ 128 Hz
  OCR0A = 120;

  // przerwanie przy porównaniu na kanale A
  TIMSK0 = _BV(OCIE0A);
}

// 16 * 8 = 128
volatile static uint8_t buffer[16];
volatile static uint8_t i = 0;

ISR(TIMER0_COMPA_vect) {
  //                                        vvvvvvv i % 8 vvvvvvv                             vvvvvvv i % 8 vvvvvvv
  buffer[i >> 3] = (buffer[i >> 3] & ~(1 << (i - ((i >> 3) << 3)))) | (!!(PIND & _BV(PD0)) << (i - ((i >> 3) << 3)));
  i *= (++i != 128);
  PORTB = (PORTB & ~_BV(PB5)) | (_BV(PB5) * !((buffer[i >> 3]) & (1 << (i - ((i >> 3) << 3)))));
}

int main()
{

  // zainicjalizuj wejścia/wyjścia
  io_init();
  // zainicjalizuj licznik
  timer0_init();
  // odmaskuj przerwania
  sei();

  DDRB |= _BV(PB5);

  set_sleep_mode(SLEEP_MODE_IDLE);
  // program testowy
  while(1) {
    sleep_mode();
  }
}