#define __AVR_ATmega328P__
#define __DELAY_BACKWARD_COMPATIBLE__

#include <avr/pgmspace.h>
#include <avr/io.h>
#include <util/delay.h>
#include <stdio.h>
#include <inttypes.h>

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

// inicjalizacja ADC
void adc_init()
{
  ADMUX   = _BV(REFS0); // referencja AVcc, wejście ADC0
  DIDR0   = _BV(ADC0D); // wyłącz wejście cyfrowe na ADC0
  // częstotliwość zegara ADC 125 kHz (16 MHz / 128)
  ADCSRA  = _BV(ADPS0) | _BV(ADPS1) | _BV(ADPS2); // preskaler 128
  ADCSRA |= _BV(ADEN);  // włącz ADC
  ADCSRA |= _BV(ADATE); // auto-trigger
  ADCSRA |= _BV(ADSC);  // start conversion
}

#define LED PB0
#define LED_DDR DDRB
#define LED_PORT PORTB

FILE uart_file;

uint16_t arr[] = {
  1, 1, 1, 1, 1, 1, 1, 2,
  2, 2, 2, 3, 3, 4, 4, 5,
  5, 6, 7, 7, 8, 9, 10, 12,
  13, 14, 16, 18, 20, 22, 25, 28,
  31, 35, 39, 43, 49, 54, 60, 67,
  75, 84, 93, 104, 116, 129, 144, 160,
  179, 199, 222, 248, 276, 307, 343, 382,
  425, 474, 528, 588, 656, 731, 814, 907};

volatile int main()
{
  uart_init();
  fdev_setup_stream(&uart_file, uart_transmit, uart_receive, _FDEV_SETUP_RW);
  stdin = stdout = stderr = &uart_file;
  adc_init();

  LED_DDR |= _BV(LED);
  uint16_t v;

  while(1) {
    while (!(ADCSRA & _BV(ADIF))); // czekaj na wynik
    ADCSRA |= _BV(ADIF); // wyczyść bit ADIF (pisząc 1!)
    v = ADC; // weź zmierzoną wartość (0..1023)
    v >>= 4;

    LED_PORT |= _BV(LED);
    _delay_us(arr[63 - v]);
    LED_PORT &= ~_BV(LED);
    _delay_us(arr[v]);
  }
}