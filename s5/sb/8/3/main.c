// L8Z3

#include "FreeRTOS.h"
#include "portmacro.h"
#include "projdefs.h"
#include "queue.h"
#include "task.h"
#include <avr/interrupt.h>
#include <avr/io.h>
#include <stdint.h>
#include <stdio.h>

#define mainLED_TASK_PRIORITY 1
#define mainSERIAL_TASK_PRIORITY 2

#define BAUD 9600                            // baudrate
#define UBRR_VALUE ((F_CPU) / 16 / (BAUD)-1) // zgodnie ze wzorem

static void vLight(void *pvParameters);
static void vSerial(void *pvParameters);

int uart_transmit(char c, FILE *stream);
int uart_receive(FILE *stream);

FILE uart_file = FDEV_SETUP_STREAM(uart_transmit, uart_receive, _FDEV_SETUP_RW);

// inicjalizacja UART
void uart_init(void) {
  // ustaw baudrate
  UBRR0 = UBRR_VALUE;
  // wyczyść rejestr UCSR0A
  UCSR0A = 0;
  // włącz odbiornik, nadajnik oraz przerwania po odbiorze i opróżnieniu
  // rejestru
  UCSR0B = _BV(RXEN0) | _BV(TXEN0) | _BV(RXCIE0) | _BV(UDRIE0);
  // ustaw format 8n1
  UCSR0C = _BV(UCSZ00) | _BV(UCSZ01);
}

QueueHandle_t tx_queue, rx_queue;

// transmisja jednego znaku
int uart_transmit(char data, FILE *stream) {
  xQueueSend(tx_queue, &data, portMAX_DELAY);
  UCSR0B |= _BV(UDRIE0);
  return 0;
}

// odczyt jednego znaku
int uart_receive(FILE *stream) {
  uint8_t data;
  xQueueReceive(rx_queue, &data, portMAX_DELAY);
  return data;
}

ISR(USART_RX_vect) {
  uint8_t data = UDR0;
  xQueueSendFromISR(rx_queue, &data, NULL);
}

ISR(USART_UDRE_vect) {
  uint8_t data;
  if (xQueueReceiveFromISR(tx_queue, &data, 0) == pdTRUE)
    UDR0 = data;
  else
    UCSR0B &= ~_BV(UDRIE0);
}

int main(void) {
  rx_queue = xQueueCreate(100, sizeof(uint8_t));
  tx_queue = xQueueCreate(100, sizeof(uint8_t));
  
  // Create task.
  xTaskHandle blink_handle;
  xTaskHandle serial_handle;

  xTaskCreate(vLight, "light", configMINIMAL_STACK_SIZE, NULL,
              mainLED_TASK_PRIORITY, &blink_handle);

  xTaskCreate(vSerial, "serial", 200, NULL, mainSERIAL_TASK_PRIORITY,
              &serial_handle);

  // Start scheduler.
  vTaskStartScheduler();

  return 0;
}

void vApplicationIdleHook(void) {}

static void vLight(void *pvParameters) {
  DDRB |= _BV(PB5);

  for (;;) {
    PORTB ^= _BV(PB5);
    vTaskDelay(100 / portTICK_PERIOD_MS);
  }
}

static void vSerial(void *pvParameters) {
  uart_init();
  stdin = stdout = stderr = &uart_file;

  uint8_t input;

  for (;;) {
    input = getchar();
    putchar(input);
    if (input == '\r')
      putchar('\n');
  }
}