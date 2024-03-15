// L8Z2

#include "FreeRTOS.h"
#include "portmacro.h"
#include "projdefs.h"
#include "task.h"
#include "queue.h"
#include <avr/io.h>
#include <stdint.h>
#include <stdio.h>

#define mainLED_TASK_PRIORITY    1
#define mainSERIAL_TASK_PRIORITY 1

#define BAUD 9600                          // baudrate
#define UBRR_VALUE ((F_CPU)/16/(BAUD)-1)   // zgodnie ze wzorem

static void vLight(void* pvParameters);
static void vSerial(void* pvParameters);

int uart_transmit(char c, FILE *stream);
int uart_receive(FILE *stream);

FILE uart_file = FDEV_SETUP_STREAM(uart_transmit, uart_receive, _FDEV_SETUP_RW);
QueueHandle_t queue;

// inicjalizacja UART
void uart_init(void)
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

int uart_transmit(char c, FILE *stream) {
  while (!(UCSR0A & _BV(UDRE0))) taskYIELD();
  UDR0 = c;
  return 0;
}

int uart_receive(FILE *stream) {
  while (!(UCSR0A & _BV(RXC0))) taskYIELD();
  return UDR0;
}

int main(void)
{
    queue = xQueueCreate(100, sizeof(uint16_t));

    // Create task.
    xTaskHandle blink_handle;
    xTaskHandle serial_handle;

    xTaskCreate
        (
         vLight,
         "light",
         configMINIMAL_STACK_SIZE,
         NULL,
         mainLED_TASK_PRIORITY,
         &blink_handle
        );

    xTaskCreate
        (
         vSerial,
         "serial",
         200,
         NULL,
         mainSERIAL_TASK_PRIORITY,
         &serial_handle
        );

    // Start scheduler.
    vTaskStartScheduler();

    return 0;
}


void vApplicationIdleHook(void)
{

}

static void vLight(void* pvParameters)
{
    DDRB |= _BV(PB5);
    uint16_t delay;

    for ( ;; )
    {
        delay = 0;
        if (xQueueReceive(queue, &delay, 0) == pdPASS)
        {
            printf("Otrzymano: %u\r\n", delay);
            PORTB |= _BV(PB5);
            vTaskDelay(delay / portTICK_PERIOD_MS);
            PORTB &= ~_BV(PB5);
            vTaskDelay(1000 / portTICK_PERIOD_MS);
        }
    }
}

static void vSerial(void* pvParameters)
{
    uart_init();
    stdin = stdout = stderr = &uart_file;

    uint16_t input;

    for ( ;; )
    {
        scanf("%u", &input);
        printf("Dodano: %u\r\n", input);
        xQueueSend(queue, &input, portMAX_DELAY);
        taskYIELD();
    }
}