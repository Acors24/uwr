// L5Z2

#define __AVR_ATmega328P__

#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>
#include <stdio.h>
#include <inttypes.h>

#define BAUD 9600                            // baudrate
#define UBRR_VALUE ((F_CPU) / 16 / (BAUD)-1) // zgodnie ze wzorem

// inicjalizacja UART
void uart_init()
{
  // ustaw baudrate
  UBRR0 = UBRR_VALUE;
  // włącz odbiornik i nadajnik
  UCSR0B = _BV(RXEN0) | _BV(TXEN0);
  // ustaw format 8n1
  UCSR0C = _BV(UCSZ00) | _BV(UCSZ01);
}

// transmisja jednego znaku
int uart_transmit(char data, FILE *stream)
{
  // czekaj aż transmiter gotowy
  while (!(UCSR0A & _BV(UDRE0)))
    ;
  UDR0 = data;
  return 0;
}

// odczyt jednego znaku
int uart_receive(FILE *stream)
{
  // czekaj aż znak dostępny
  while (!(UCSR0A & _BV(RXC0)))
    ;
  return UDR0;
}

FILE uart_file;

void io_init()
{
  // pull-up na PD3 (INT1)
  PORTD |= _BV(PD3);
  // zbocze opadające na pinie INT1
  EICRA |= _BV(ISC11);
  EIMSK |= _BV(INT1);
}

// przycisk
ISR(INT1_vect)
{
  ADMUX = _BV(REFS0);
  // częstotliwość zegara ADC 125 kHz (16 MHz / 128)
  ADCSRA  = _BV(ADPS2) | _BV(ADPS1) | _BV(ADPS0); // preskaler 128
  ADCSRA |= _BV(ADATE) | _BV(ADSC) | _BV(ADEN);
  ADCSRA |= _BV(ADIE); // interrupt enable
  DIDR0   = _BV(ADC0D); // wyłącz wejście cyfrowe na ADC0
}

volatile static int32_t v;

ISR(ADC_vect)
{
  v = ADC;
  v = (v * -10000) / 1024 + 10000;
}

int main()
{
  // zainicjalizuj UART
  uart_init();
  // skonfiguruj strumienie wejścia/wyjścia
  fdev_setup_stream(&uart_file, uart_transmit, uart_receive, _FDEV_SETUP_RW);
  stdin = stdout = stderr = &uart_file;
  // zainicjalizuj licznik
  // timer1_init();
  
  // zainicjalizuj przerwania
  io_init();
  
  // odmaskuj przerwania
  sei();

  // program testowy
  while (1)
  {
    printf("%d Ω\r\n", v);
    // printf("ADC: %d\r\n", ADC);
    _delay_ms(500);
  }
}