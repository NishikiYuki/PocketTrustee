#include "ll_rfid.hpp"
#include "driver_w25q16.hpp"
#include "guiv2.hpp"
#include "main.h"
#include "FreeRTOS.h"
#include "task.h"
#include <bits/this_thread_sleep.h>

extern "C" {
void StartDefaultTask(void const* argument);
void vApplicationStackOverflowHook(TaskHandle_t xTask, char* pcTaskName);
void vApplicationMallocFailedHook(void);
}

void HAL_Delay(uint32_t Delay) {
    vTaskDelay(Delay);
}

void StartDefaultTask(void const* argument) {
    for (;;) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {
    if (GPIO_Pin == FLG_CHG_Pin) {

    }
    else if (GPIO_Pin == FLG_USB_Pin) {
    }
    else {
        switch (GPIO_Pin) {
        case KUP_Pin:
            if (KUP_GPIO_Port->IDR & KUP_Pin) break;

            break;
        case KDN_Pin:
            if (KDN_GPIO_Port->IDR & KDN_Pin) break;

            break;
        case KEN_Pin:
            if (KEN_GPIO_Port->IDR & KEN_Pin) break;

            break;
        default:
            break;
        }
    }
}