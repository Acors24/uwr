// L7Z3

#include "i2c.c"
#include <avr/io.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <util/delay.h>

#define BAUD 9600                            // baudrate
#define UBRR_VALUE ((F_CPU) / 16 / (BAUD)-1) // zgodnie ze wzorem

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

#define i2cCheck(code, msg)                                                    \
  if ((TWSR & 0xf8) != (code)) {                                               \
    printf(msg " failed, status: %.2x\r\n", TWSR & 0xf8);                      \
    i2cReset();                                                                \
    continue;                                                                  \
  }

FILE uart_file;

/**/ /**/ /**/ /**/ /**/ /**/ /**/ /**/ /**/ /**/ /**/
/**/ const uint8_t eeprom_addr = 0xa0;            /**/
/**/ const uint8_t rtc_addr = 0xd0;               /**/
/**/ /**/ /**/ /**/ /**/ /**/ /**/ /**/ /**/ /**/ /**/

void read_bytes(uint8_t addr, uint8_t data[3]) {
  i2cStart();
  i2cSend(rtc_addr | ((addr & 0x100) >> 7));
  i2cSend(addr & 0xff);
  i2cStart();
  i2cSend(rtc_addr | 0x1 | ((addr & 0x100) >> 7));
  data[0] = i2cReadAck();
  data[1] = i2cReadAck();
  data[2] = i2cReadNoAck();
  i2cStop();
}

void write_bytes(uint8_t addr, uint8_t data[3]) {
  i2cStart();
  i2cSend(rtc_addr | ((addr & 0x100) >> 7));
  i2cSend(addr & 0xff);
  i2cSend(data[0]);
  i2cSend(data[1]);
  i2cSend(data[2]);
  i2cStop();
  _delay_ms(10);
}

void read_input(char *input, const uint8_t BUF_SIZE) {
  int ch;
  uint8_t i = 0;
  while (i < BUF_SIZE)
    input[i++] = '\0';
  input[i] = '\0';
  i = 0;

  while ((ch = getchar()) != '\r') {
    putchar(ch);
    input[i++] = ch;
    i %= BUF_SIZE;
  }
  input[i] = '\0';
  putchar('\r');
  putchar('\n');
}

int hex_to_dec(int ch) {
  if ('0' <= ch && ch <= '9')
    return ch - '0';
  else
    return ch - 'A' + 10;
}

int main() {
  // zainicjalizuj UART
  uart_init();
  // skonfiguruj strumienie wejścia/wyjścia
  fdev_setup_stream(&uart_file, uart_transmit, uart_receive, _FDEV_SETUP_RW);
  stdin = stdout = stderr = &uart_file;
  // zainicjalizuj I2C
  i2cInit();
  // program testowy

  const uint8_t BUF_SIZE = 30;
  char com[BUF_SIZE];
  uint8_t data[3] = {0, 0, 0};

  while (1) {
    scanf("%s", com);
    printf("%s\r\n", com);
    if (!strcmp(com, "date")) {
      read_bytes(4, data);
      uint8_t date = data[0];
      uint8_t month = data[1];
      uint8_t year = data[2];

      date = ((date & 0x30) >> 4) * 10 + (date & 0x0F);
      uint8_t century = (month & 0x80) >> 7;
      month = ((month & 0x10) >> 4) * 10 + (month & 0x0F);
      year = ((year & 0xF0) >> 4) * 10 + (year & 0x0F);

      printf("%02hhu-%02hhu-%hhu%02hhu\r\n", date, month, 19 + century, year);
      // printf("%x-%x-%x\r\n", date, month, year);
    } else if (!strcmp(com, "time")) {
      read_bytes(0, data);
      uint8_t second = data[0];
      uint8_t minute = data[1];
      uint8_t hour = data[2];

      second = ((second & 0x70) >> 4) * 10 + (second & 0x0F);
      minute = ((minute & 0x70) >> 4) * 10 + (minute & 0x0F);
      hour = ((hour & 0x30) >> 4) * 10 + (hour & 0x0F);

      // printf("%02x:%02x:%02x\r\n", hour, minute, second);
      printf("%02hhu:%02hhu:%02hhu\r\n", hour, minute, second);
    } else if (!strcmp(com, "set")) {
      scanf("%s", com);
      if (!strcmp(com, "date")) {
        uint8_t date, month, century;
        uint16_t year;
        scanf("%hhu-%hhu-%u", &date, &month, &year);

        date = ((date / 10) << 4) | (date % 10);
        century = year > 1999;
        month = ((month / 10) << 4) | (month % 10) | (century << 7);
        year %= 100;
        year = ((year / 10) << 4) | (year % 10);

        uint8_t data[3] = {date, month, year};
        write_bytes(4, data);
      } else if (!strcmp(com, "time")) {
        uint8_t hour, minute, second;
        scanf("%hhu:%hhu:%hhu", &hour, &minute, &second);

        hour = (((hour / 10) << 4) | (hour % 10)) & ~(1 << 6);
        minute = ((minute / 10) << 4) | (minute % 10);
        second = ((second / 10) << 4) | (second % 10);

        uint8_t data[3] = {second, minute, hour};
        write_bytes(0, data);
      }
    }
  }
}
