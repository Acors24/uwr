// L11Z1

#include <avr/io.h>
#include <stdint.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>

void adc_init() {
  ADMUX = _BV(REFS0);
  // częstotliwość zegara ADC 125 kHz (16 MHz / 128)
  ADCSRA  = _BV(ADPS2) | _BV(ADPS1) | _BV(ADPS0); // preskaler 128
  ADCSRA |= _BV(ADSC) | _BV(ADEN) | _BV(ADIE);
  DIDR0   = _BV(ADC0D); // wyłącz wejście cyfrowe na ADC0

  set_sleep_mode(SLEEP_MODE_IDLE);
}

uint16_t adc_v = 0;
ISR(ADC_vect) {
  adc_v = ADC;
}

uint16_t adsc(uint8_t mux) {
  ADMUX = (ADMUX & 0xF0) | (mux & 0x0F);
  ADCSRA |= _BV(ADSC);
  sleep_mode();

  return adc_v;
}

void timer1_init()
{
  // ustaw tryb licznika
  // COM1A = 10   -- clear on compare match
  // WGM1  = 1110 -- fast PWM top=ICR1
  // CS1   = 011  -- /64
  TCCR1A = _BV(COM1A1) | _BV(WGM11);// | _BV(WGM10);
  TCCR1B = _BV(WGM13) | _BV(WGM12) | _BV(CS11) | _BV(CS10);

  // f = 16e6 / N / (1 + TOP)
  // f * (1 + TOP) = 16e6 / N
  // 1 + TOP = 16e6 / N / f
  // TOP = 16e6 / N / f - 1
  // 499 = 16e6 / 64 / 500 - 1
  ICR1 = 499;

  // ustaw pin OC1A (PB1) jako wyjście
  DDRB |= _BV(PB1);
}

int main()
{
  adc_init();
  timer1_init();

  sei();

  while(1) {
    OCR1A = (adsc(0) / 1023.0) * ICR1;
  }
}

