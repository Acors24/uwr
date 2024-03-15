// L8Z1

#include "FreeRTOS.h"
#include "portmacro.h"
#include "task.h"
#include <avr/io.h>
#include <stdint.h>
#include <stdio.h>

#define mainLED_TASK_PRIORITY   2
#define mainSERIAL_TASK_PRIORITY 1

static void vBar(void* pvParameters);
static void vButton(void* pvParameters);

int main(void)
{
    // Create task.
    xTaskHandle blink_handle;
    xTaskHandle button_handle;

    xTaskCreate
        (
         vBar,
         "bar",
         configMINIMAL_STACK_SIZE,
         NULL,
         mainLED_TASK_PRIORITY,
         &blink_handle
        );

    xTaskCreate
        (
         vButton,
         "button",
         configMINIMAL_STACK_SIZE,
         NULL,
         mainSERIAL_TASK_PRIORITY,
         &button_handle
        );

    // Start scheduler.
    vTaskStartScheduler();

    return 0;
}


void vApplicationIdleHook(void)
{

}

static void vBar(void* pvParameters)
{
    DDRD |= 255;
    uint8_t i = 0, dir = 1;

    for ( ;; )
    {
        PORTD = ((1 * dir) << i) | ((128 * (1 - dir)) >> i);
        if (++i == 7)
        {
            i = 0;
            dir = !dir;
        }
        vTaskDelay(67.5 / portTICK_PERIOD_MS);
    }
}

static void vButton(void* pvParameters)
{
    // 16 * 8 = 128
    volatile static uint8_t buffer[16];
    volatile static uint8_t i = 0;
    DDRB |= _BV(PB5);
    PORTB |= _BV(PB4);

    for ( ;; )
    {
        buffer[i >> 3] = (buffer[i >> 3] & ~(1 << (i & 7))) | (!!(PINB & _BV(PB4)) << (i & 7));
        i *= i++ != 100;
        PORTB = (PORTB & ~_BV(PB5)) | (_BV(PB5) * !((buffer[i >> 3]) & (1 << (i & 7))));
        vTaskDelay(10 / portTICK_PERIOD_MS);
    }
}