// L13Z2

#include <avr/interrupt.h>
#include <avr/sleep.h>
#include <avr/io.h>
#include <stdint.h>
#include <stdio.h>
#include <util/delay.h>

#define BAUD 9600                            // baudrate
#define UBRR_VALUE ((F_CPU) / 16 / (BAUD)-1) // zgodnie ze wzorem

// inicjalizacja UART
void uart_init() {
  // ustaw baudrate
  UBRR0 = UBRR_VALUE;
  // włącz odbiornik i nadajnik
  UCSR0B = _BV(RXEN0) | _BV(TXEN0);
  // ustaw format 8n1
  UCSR0C = _BV(UCSZ00) | _BV(UCSZ01);
}

// transmisja jednego znaku
int uart_transmit(char data, FILE *stream) {
  // czekaj aż transmiter gotowy
  while (!(UCSR0A & _BV(UDRE0)))
    ;
  UDR0 = data;
  return 0;
}

// odczyt jednego znaku
int uart_receive(FILE *stream) {
  // czekaj aż znak dostępny
  while (!(UCSR0A & _BV(RXC0)))
    ;
  return UDR0;
}

FILE uart_file;

void adc_init() {
  ADMUX = _BV(REFS0);
  // częstotliwość zegara ADC 125 kHz (16 MHz / 128)
  //ADCSRA = _BV(ADPS2) | _BV(ADPS1) | _BV(ADPS0); // preskaler 128
  ADCSRA = _BV(ADPS2); // inny preskaler
  ADCSRA |= _BV(ADEN);
  ADCSRA |= _BV(ADIE); // interrupt enable
  DIDR0  = _BV(ADC1D); // wyłącz wejście cyfrowe na ADC1
  DIDR0 |= _BV(ADC2D); // wyłącz wejście cyfrowe na ADC2
  set_sleep_mode(SLEEP_MODE_IDLE);
}

volatile uint16_t adc_v;
ISR(ADC_vect) {
  adc_v = ADC;
}

int main() {
  // zainicjalizuj UART
  uart_init();
  // skonfiguruj strumienie wejścia/wyjścia
  fdev_setup_stream(&uart_file, uart_transmit, uart_receive, _FDEV_SETUP_RW);
  stdin = stdout = stderr = &uart_file;
  // program testowy
  adc_init();
  sei();

  ADMUX |= _BV(MUX1);
  //  1 ~>  2 ~>  1
  // 01 ~> 10 ~> 01
  DDRB |= _BV(PB5);

  printf(" ON        | OFF\r\n");
  printf(" ADC1 ADC2 | ADC1 ADC2\r\n");
  uint16_t v_in_x10 = 0;

  while (1) {
    ADMUX ^= _BV(MUX1) | _BV(MUX0);
    PORTB ^= _BV(PB5);
    ADCSRA |= _BV(ADSC);
    sleep_mode();
    // ADC = VIN * 1024 / VREF
    // ADC * VREF = VIN * 1024
    // ADC * VREF / 1024 = VIN
    v_in_x10 = adc_v * 50 / 1024;

    // printf("\r %4hu", adc_v);
    printf("\r %1hu.%1huV", v_in_x10 / 10, v_in_x10 % 10);

    ADMUX ^= _BV(MUX1) | _BV(MUX0);
    ADCSRA |= _BV(ADSC);
    sleep_mode();
    v_in_x10 = adc_v * 50 / 1024;

    // printf(" %4hu", adc_v);
    printf(" %1hu.%1huV", v_in_x10 / 10, v_in_x10 % 10);

    ADMUX ^= _BV(MUX1) | _BV(MUX0);
    PORTB ^= _BV(PB5);
    ADCSRA |= _BV(ADSC);
    sleep_mode();
    v_in_x10 = adc_v * 50 / 1024;

    // printf("   %4hu", adc_v);
    printf("   %1hu.%1huV", v_in_x10 / 10, v_in_x10 % 10);

    ADMUX ^= _BV(MUX1) | _BV(MUX0);
    ADCSRA |= _BV(ADSC);
    sleep_mode();
    v_in_x10 = adc_v * 50 / 1024;

    // printf(" %4hu", adc_v);
    printf(" %1hu.%1huV", v_in_x10 / 10, v_in_x10 % 10);

    _delay_ms(50);
  }
}
