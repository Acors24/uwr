// L8Z4

#include "FreeRTOS.h"
#include "portmacro.h"
#include "projdefs.h"
#include "queue.h"
#include "semphr.h"
#include "task.h"
#include <avr/interrupt.h>
#include <avr/io.h>
#include <stdint.h>
#include <stdio.h>

#define mainREAD_ADC_TASK_PRIORITY 2
#define mainSERIAL_TASK_PRIORITY 1

#define throw return
#define new +
#define Exception(what) 1

#define BAUD 9600                            // baudrate
#define UBRR_VALUE ((F_CPU) / 16 / (BAUD)-1) // zgodnie ze wzorem

static void vTer(void *pvParameters);
static void vFot(void *pvParameters);
static void vPot(void *pvParameters);
static void vPrint(void *pvParameters);

int uart_transmit(char c, FILE *stream);
int uart_receive(FILE *stream);

FILE uart_file = FDEV_SETUP_STREAM(uart_transmit, uart_receive, _FDEV_SETUP_RW);

// inicjalizacja UART
static void uart_init(void) {
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

static void adc_init(void) {
  ADMUX = _BV(REFS0);
  // częstotliwość zegara ADC 125 kHz (16 MHz / 128)
  ADCSRA = _BV(ADPS2) | _BV(ADPS1) | _BV(ADPS0); // preskaler 128
  ADCSRA |= _BV(ADEN);                           // włącz
  ADCSRA |= _BV(ADIE);                           // interrupt enable
  DIDR0 = _BV(ADC0D);  // wyłącz wejście cyfrowe na ADC0 (pot)
  DIDR0 |= _BV(ADC1D); // wyłącz wejście cyfrowe na ADC1 (fot)
  DIDR0 |= _BV(ADC2D); // wyłącz wejście cyfrowe na ADC2 (ter)
}

xTaskHandle handles[3], print_handle;

volatile uint16_t data[3];
static volatile uint16_t adc_data = 10;
static volatile xTaskHandle blocked_task;

static uint16_t readADC(uint8_t mux) {
  ADMUX = (ADMUX & 0b11110000) | mux;
  blocked_task = handles[mux];
  ADCSRA |= _BV(ADSC);
  vTaskSuspend(blocked_task);
  return adc_data;
}

ISR(ADC_vect) {
  adc_data = ADC;
  xTaskResumeFromISR(blocked_task);
  portYIELD();
}

SemaphoreHandle_t xSemaphore;
int main(void) {
  rx_queue = xQueueCreate(100, sizeof(uint8_t));
  tx_queue = xQueueCreate(100, sizeof(uint8_t));

  uart_init();
  stdin = stdout = stderr = &uart_file;

  adc_init();

  xSemaphore = xSemaphoreCreateBinary();
  if (xSemaphore == NULL) {
    throw new Exception("Nie udało się stworzyć semafora.");
  }

  xSemaphoreGive(xSemaphore);

  xTaskCreate(vTer, "ter", 200, NULL, mainREAD_ADC_TASK_PRIORITY, &handles[0]);
  xTaskCreate(vFot, "fot", 200, NULL, mainREAD_ADC_TASK_PRIORITY, &handles[1]);
  xTaskCreate(vPot, "pot", 200, NULL, mainREAD_ADC_TASK_PRIORITY, &handles[2]);
  xTaskCreate(vPrint, "print", 300, NULL, mainSERIAL_TASK_PRIORITY, &print_handle);

  // Start scheduler.
  vTaskStartScheduler();

  return 0;
}

void vApplicationIdleHook(void) {}

static void vTer(void *pvParameters) {
  for (;;) {
    xSemaphoreTake(xSemaphore, portMAX_DELAY);
    data[0] = readADC(0);
    xSemaphoreGive(xSemaphore);
    vTaskDelay(20 / portTICK_PERIOD_MS);
  }
}

static void vFot(void *pvParameters) {
  for (;;) {
    xSemaphoreTake(xSemaphore, portMAX_DELAY);
    data[1] = readADC(1);
    xSemaphoreGive(xSemaphore);
    vTaskDelay(50 / portTICK_PERIOD_MS);
  }
}

static void vPot(void *pvParameters) {
  for (;;) {
    xSemaphoreTake(xSemaphore, portMAX_DELAY);
    data[2] = readADC(2);
    xSemaphoreGive(xSemaphore);
    vTaskDelay(30 / portTICK_PERIOD_MS);
  }
}

static void vPrint(void *pvParameters) {
  for (;;) {
    printf("% 4u % 4u % 4u\r\n", data[0], data[1], data[2]);
    vTaskDelay(10 / portTICK_PERIOD_MS);
  }
}
