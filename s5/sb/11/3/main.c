// L11Z3

#include <avr/io.h>
#include <stdint.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>
#include <stdio.h>

#define BAUD 9600                          // baudrate
#define UBRR_VALUE ((F_CPU)/16/(BAUD)-1)   // zgodnie ze wzorem

// inicjalizacja UART
void uart_init()
{
  // ustaw baudrate
  UBRR0 = UBRR_VALUE;
  // wyczyść rejestr UCSR0A
  UCSR0A = 0;
  // włącz odbiornik i nadajnik
  UCSR0B = _BV(RXEN0) | _BV(TXEN0);
  // ustaw format 8n1
  UCSR0C = _BV(UCSZ00) | _BV(UCSZ01);
}

// transmisja jednego znaku
int uart_transmit(char data, FILE *stream)
{
  // czekaj aż transmiter gotowy
  while(!(UCSR0A & _BV(UDRE0)));
  UDR0 = data;
  return 0;
}

// odczyt jednego znaku
int uart_receive(FILE *stream)
{
  // czekaj aż znak dostępny
  while (!(UCSR0A & _BV(RXC0)));
  return UDR0;
}

FILE uart_file;

#define LEFT_TURN  PB4
#define RIGHT_TURN PB3

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
  // COM1B = 10   -- clear on compare match
  // WGM1  = 1110 -- fast PWM top=ICR1
  // CS1   = 011  -- /64
  TCCR1A = _BV(COM1B1) | _BV(WGM11);// | _BV(WGM10);
  TCCR1B = _BV(WGM13) | _BV(WGM12) | _BV(CS11) | _BV(CS10);

  // f = 16e6 / N / (1 + TOP)
  // f * (1 + TOP) = 16e6 / N
  // 1 + TOP = 16e6 / N / f
  // TOP = 16e6 / N / f - 1
  // 499 = 16e6 / 64 / 500 - 1
  ICR1 = 499;

  // ustaw pin OC1B (PB2) jako wyjście
  DDRB |= _BV(PB2);
}

int main()
{
  // zainicjalizuj UART
  uart_init();
  // skonfiguruj strumienie wejścia/wyjścia
  fdev_setup_stream(&uart_file, uart_transmit, uart_receive, _FDEV_SETUP_RW);
  stdin = stdout = stderr = &uart_file;

  adc_init();
  timer1_init();

  sei();

  DDRB |= _BV(LEFT_TURN);
  DDRB |= _BV(RIGHT_TURN);

  int16_t pot_v;

  while(1) {
    pot_v = adsc(0) - 512;
    if (pot_v < 0) {
      PORTB |= _BV(LEFT_TURN);
      PORTB &= ~_BV(RIGHT_TURN);
      pot_v *= -1;
      pot_v -= 1;
    } else {
      PORTB &= ~_BV(LEFT_TURN);
      PORTB |= _BV(RIGHT_TURN);
    }
    pot_v *= 2;
    OCR1B = (pot_v / 1023.0) * ICR1;
  }
}

