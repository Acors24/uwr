// L4Z2

#define __AVR_ATmega328P__
#define __DELAY_BACKWARD_COMPATIBLE__

#include <avr/io.h>
#include <util/delay.h>
#include <stdio.h>
#include <inttypes.h>

void adc_init()
{
  ADMUX   = _BV(REFS1) | _BV(REFS0);
  ADCSRA  = _BV(ADPS2) | _BV(ADPS1) | _BV(ADPS0);
  ADCSRA |= _BV(ADEN);
  ADCSRA |= _BV(ADATE);
  ADCSRA |= _BV(ADSC);
}

#include "exp.h"

void timer1_init()
{
  // ustaw tryb licznika
  // COM1A = 11   -- inverting mode
  // WGM1  = 0111 -- fast PWM 10-bit top=0x3FF
  // CS1   = 001  -- brak prescalera
  TCCR1A = _BV(COM1A1) | _BV(COM1A0) | _BV(WGM11) | _BV(WGM10);
  TCCR1B = _BV(WGM12) | _BV(CS10);
  // ustaw pin OC1A (PB1) jako wyjście
  DDRB |= _BV(PB1);
}

int main()
{
  // uruchom licznik
  timer1_init();
  adc_init();
  
  while (1) {
    OCR1A = func[1023 - ADC];
  }
}