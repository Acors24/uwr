// L4Z1

#define __AVR_ATmega328P__
#define __DELAY_BACKWARD_COMPATIBLE__

#include <avr/io.h>
#include <util/delay.h>
#include <stdio.h>
#include <inttypes.h>
#include <stdlib.h>
#include <time.h>

#define BAUD 9600                            // baudrate
#define UBRR_VALUE ((F_CPU) / 16 / (BAUD)-1) // zgodnie ze wzorem

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

void timer1_init()
{
  // ustaw tryb licznika
  // WGM1  = 0000 -- normal
  // CS1   = 001  -- prescaler 1
  TCCR1B = _BV(CS10);
}
FILE uart_file;

int main()
{
  srand(time(0));

  uart_init();
  fdev_setup_stream(&uart_file, uart_transmit, uart_receive, _FDEV_SETUP_RW);
  stdin = stdout = stderr = &uart_file;

  timer1_init();

  int16_t a, b;

  volatile int8_t  int8_t_a,  int8_t_b;
  volatile int16_t int16_t_a, int16_t_b;
  volatile int32_t int32_t_a, int32_t_b;
  volatile int64_t int64_t_a, int64_t_b;
  volatile float   float_a,   float_b;

  while (1)
  {
    // {
    //   int8_t_a  = 2; int8_t_b  = -5;
    //   int16_t_a = 1234; int16_t_b = -2345;
    //   int32_t_a = 127365; int32_t_b = -96252;
    //   int64_t_a = 20867293475; int64_t_b = -9823681752;
    //   float_a   = 45.2; float_b   = 75.1;
    // }
    {
      int8_t_a  = rand(); int8_t_b  = rand();
      int16_t_a = rand(); int16_t_b = rand();
      int32_t_a = rand(); int32_t_b = rand();
      int64_t_a = rand(); int64_t_b = rand();
      float_a   = rand(); float_b   = rand();
    }

    {
      printf("Dodawanie:\r\n");

      a = TCNT1; // wartość licznika przed czekaniem
      int8_t_a = int8_t_a + int8_t_b;
      // (volatile int8_t)2 + (volatile int8_t)1;
      b = TCNT1; // wartość licznika po czekaniu
      printf("\tint8_t:  %" PRIu16 " cykli\r\n", b - a);

      a = TCNT1; // wartość licznika przed czekaniem
      int16_t_a = int16_t_a + int16_t_b;
      b = TCNT1; // wartość licznika po czekaniu
      printf("\tint16_t: %" PRIu16 " cykli\r\n", b - a);

      a = TCNT1; // wartość licznika przed czekaniem
      int32_t_a = int32_t_a + int32_t_b;
      b = TCNT1; // wartość licznika po czekaniu
      printf("\tint32_t: %" PRIu16 " cykli\r\n", b - a);

      a = TCNT1; // wartość licznika przed czekaniem
      int64_t_a = int64_t_a + int64_t_b;
      b = TCNT1; // wartość licznika po czekaniu
      printf("\tint64_t: %" PRIu16 " cykli\r\n", b - a);

      a = TCNT1; // wartość licznika przed czekaniem
      float_a = float_a + float_b;
      b = TCNT1; // wartość licznika po czekaniu
      printf("\tfloat:   %" PRIu16 " cykli\r\n", b - a);

      printf("\r\n");
    }

    {
      printf("Mnożenie:\r\n");

      a = TCNT1; // wartość licznika przed czekaniem
      int8_t_a = int8_t_a *int8_t_b;
      b = TCNT1; // wartość licznika po czekaniu
      printf("\tint8_t:  %" PRIu16 " cykli\r\n", b - a);

      a = TCNT1; // wartość licznika przed czekaniem
      int16_t_a = int16_t_a *int16_t_b;
      b = TCNT1; // wartość licznika po czekaniu
      printf("\tint16_t: %" PRIu16 " cykli\r\n", b - a);

      a = TCNT1; // wartość licznika przed czekaniem
      int32_t_a = int32_t_a *int32_t_b;
      b = TCNT1; // wartość licznika po czekaniu
      printf("\tint32_t: %" PRIu16 " cykli\r\n", b - a);

      a = TCNT1; // wartość licznika przed czekaniem
      int64_t_a = int64_t_a *int64_t_b;
      b = TCNT1; // wartość licznika po czekaniu
      printf("\tint64_t: %" PRIu16 " cykli\r\n", b - a);

      a = TCNT1; // wartość licznika przed czekaniem
      float_a = float_a *float_b;
      b = TCNT1; // wartość licznika po czekaniu
      printf("\tfloat:   %" PRIu16 " cykli\r\n", b - a);

      printf("\r\n");
    }

    {
      printf("Dzielenie:\r\n");

      a = TCNT1; // wartość licznika przed czekaniem
      int8_t_a = int8_t_a / int8_t_b;
      b = TCNT1; // wartość licznika po czekaniu
      printf("\tint8_t:  %" PRIu16 " cykli\r\n", b - a);

      a = TCNT1; // wartość licznika przed czekaniem
      int16_t_a = int16_t_a / int16_t_b;
      b = TCNT1; // wartość licznika po czekaniu
      printf("\tint16_t: %" PRIu16 " cykli\r\n", b - a);

      a = TCNT1; // wartość licznika przed czekaniem
      int32_t_a = int32_t_a / int32_t_b;
      b = TCNT1; // wartość licznika po czekaniu
      printf("\tint32_t: %" PRIu16 " cykli\r\n", b - a);

      a = TCNT1; // wartość licznika przed czekaniem
      int64_t_a = int64_t_a / int64_t_b;
      b = TCNT1; // wartość licznika po czekaniu
      printf("\tint64_t: %" PRIu16 " cykli\r\n", b - a);

      a = TCNT1; // wartość licznika przed czekaniem
      float_a = float_a / float_b;
      b = TCNT1; // wartość licznika po czekaniu
      printf("\tfloat:   %" PRIu16 " cykli\r\n", b - a);

      printf("\r\n");
    }

    _delay_ms(2000);
  }
}
