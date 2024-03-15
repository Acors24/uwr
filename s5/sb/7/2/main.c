// L7Z2

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

/**//**//**//**//**//**//**//**//**//**//**/
/**/ const uint8_t eeprom_addr = 0xa0;  /**/
/**//**//**//**//**//**//**//**//**//**//**/

void read_bytes(uint8_t addr, uint8_t length) {
  do {
    i2cStart();
    i2cSend(eeprom_addr | ((addr & 0x100) >> 7));
    i2cSend(addr & 0xff);
    i2cStart();
    i2cSend(eeprom_addr | 0x1 | ((addr & 0x100) >> 7));
    uint8_t checksum = length + addr;
    putchar(':');
    printf("%.2X", length);
    printf("00%.2X", addr);
    printf("00");
    int i;
    for (i = 0; i < length - 1; i++) {
      uint8_t data = i2cReadAck();
      printf("%.2X", data);
      checksum += data;
    }
    uint8_t data = i2cReadNoAck();
    printf("%.2X", data);
    checksum += data;
    printf("%.2X\r\n", (uint8_t)~checksum + 1);
    printf(":00000001FF\r\n");
    i2cStop();
  } while (0);
}

void write_bytes(uint8_t addr, uint8_t *data, uint8_t length) {
  do {
    i2cStart();
    i2cSend(eeprom_addr | ((addr & 0x100) >> 7));
    i2cSend(addr & 0xff);
    uint8_t i = 0;
    while (i < length) {
      i2cSend(data[i++]);
    }
    i2cStop();
    _delay_ms(10);
  } while (0);
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

  const uint8_t BUF_SIZE = 100;
  char com[BUF_SIZE];

  while (1) {
    read_input(com, BUF_SIZE);

    if (strstr(com, "read") != NULL) {
      char *arg_begin = com + 5;
      char *space_pos = strchrnul(arg_begin, ' ');
      int arg_len = space_pos - arg_begin;
      char addr_buf[arg_len + 1];
      strncpy(addr_buf, arg_begin, arg_len);
      addr_buf[arg_len] = '\0';
      uint8_t addr = atoi(addr_buf);
      uint8_t length = 1;

      if (space_pos != com + strlen(com)) {
        arg_begin = space_pos + 1;
        space_pos = strchrnul(arg_begin, ' ');
        arg_len = space_pos - arg_begin;
        char length_buf[arg_len + 1];
        strncpy(length_buf, arg_begin, arg_len);
        length_buf[arg_len] = '\0';
        length = atoi(length_buf);
      }
      read_bytes(addr, length);
    } else if (strstr(com, "write") != NULL) {
      printf("com: '%s'\r\n", com);
      char *arg_begin = com + 6;
      char *space_pos = strchrnul(arg_begin, ' ');
      int arg_len = space_pos - arg_begin;
      if (arg_len != 0) {
        char addr_buf[arg_len + 1];
        strncpy(addr_buf, arg_begin, arg_len);
        addr_buf[arg_len] = '\0';
        uint8_t addr = atoi(addr_buf);

        arg_begin = space_pos + 1;
        space_pos = strchrnul(arg_begin, ' ');
        arg_len = space_pos - arg_begin;
        char value_buf[arg_len + 1];
        strncpy(value_buf, arg_begin, arg_len);
        value_buf[arg_len] = '\0';
        uint8_t value = atoi(value_buf);

        write_bytes(addr, &value, 1);
      } else {
        read_input(com, BUF_SIZE);
        while (strcmp(com, ":00000001FF")) {
          char *ptr = com + 1; // ":"
          uint8_t byte_count0 = hex_to_dec(*ptr++);
          uint8_t byte_count1 = hex_to_dec(*ptr++);
          const uint8_t byte_count = byte_count0 * 16 + byte_count1;

          ptr += 2; // 1 bajt adresu zamiast 2
          uint8_t address0 = hex_to_dec(*ptr++);
          uint8_t address1 = hex_to_dec(*ptr++);
          const uint8_t addr = address0 * 16 + address1;

          ptr += 2; // record type: 00
          uint8_t data[byte_count];
          uint8_t i = 0;
          while (i < byte_count) {
            uint8_t data_byte0 = hex_to_dec(*ptr++);
            uint8_t data_byte1 = hex_to_dec(*ptr++);
            data[i++] = data_byte0 * 16 + data_byte1;
          }

          // printf("count: %d; addr: %d\r\n", byte_count, addr);

          write_bytes(addr, data, byte_count);

          read_input(com, BUF_SIZE);
        }
      }
    }
  }
}
