// L7Z4

#include <avr/interrupt.h>
#include <avr/io.h>
#include <avr/sleep.h>
#include <stdio.h>
#include <util/delay.h>

#define BAUD 9600                            // baudrate
#define UBRR_VALUE ((F_CPU) / 16 / (BAUD)-1) // zgodnie ze wzorem

#define MASTER_MOSI PD5
#define MASTER_MISO PD4
#define MASTER_SCK PD6
#define MASTER_SS PD7

#define SLAVE_MOSI PB3
#define SLAVE_MISO PB4
#define SLAVE_SCK PB5
#define SLAVE_SS PB2

inline void timer0_init() {
  // PWM, top=0xFF, clear on compare
  TCCR0A = _BV(COM0A1) | _BV(WGM01) | _BV(WGM00);
  // 16e6 / 1024 / 256 ≈ 61 Hz
  // prescaler /1024
  TCCR0B = _BV(CS02) | _BV(CS00);
  OCR0A = 127;
  // przerwanie po porównaniu na kanale A
  TIMSK0 = _BV(OCIE0A);
}

volatile uint8_t master_spdr = 0;
volatile uint8_t master_data = 0;
volatile uint8_t master_i = 0;
ISR(TIMER0_COMPA_vect) {
  PORTD = (PORTD & ~_BV(MASTER_MOSI)) | ((master_spdr & 1) << MASTER_MOSI);
  master_spdr >>= 1;
  master_spdr = (master_spdr & ~0x80) | (!!(PIND & _BV(MASTER_MISO)) << 7);
  if (++master_i == 8) {
    printf("%03d ", master_spdr);
    master_data++;
    master_i = 0;
    master_spdr = master_data;
  }
}

inline void spi_init() {
  // włącz SPI; włącz przerwanie po transferze;
  // inny data order; inna faza i polaryzacja zegara
  SPCR = _BV(SPE) | _BV(SPIE) | _BV(DORD) | _BV(CPHA) | _BV(CPOL);
  DDRB |= _BV(SLAVE_MISO);
}

volatile uint8_t slave_spdr;
ISR(SPI_STC_vect) {
  slave_spdr = SPDR;
  SPDR = slave_spdr * 2;
  printf("%03d\r\n", slave_spdr);
}

// inicjalizacja UART
void uart_init() {
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

int main() {
  uart_init();
  fdev_setup_stream(&uart_file, uart_transmit, uart_receive, _FDEV_SETUP_RW);
  stdin = stdout = stderr = &uart_file;

  spi_init();

  DDRD |= _BV(MASTER_MOSI);
  DDRD |= _BV(MASTER_SCK);
  DDRD |= _BV(MASTER_SS);

  timer0_init();

  sei();
  // set_sleep_mode(SLEEP_MODE_IDLE);

  while (1) {
    // sleep_mode();
  }
}
