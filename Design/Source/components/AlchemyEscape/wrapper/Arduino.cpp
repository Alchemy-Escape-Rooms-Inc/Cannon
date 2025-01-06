#include "Arduino.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

extern SerialWrapper Serial;

void delay(size_t ms)
{
    vTaskDelay(pdMS_TO_TICKS(ms));
}