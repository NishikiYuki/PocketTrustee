#include "main.h"
#include "FreeRTOS.h"
#include "task.h"
#include "app_ptt.hpp"

extern "C" {
void StartDefaultTask(void const*);
void vApplicationStackOverflowHook(TaskHandle_t xTask, char* pcTaskName);
void vApplicationMallocFailedHook(void);
}

void HAL_Delay(uint32_t Delay) {
    vTaskDelay(Delay);
}

void StartGuiTask(void const*) {
    auto& app = PocketTrustee::App::InitOrGetApp();
    app.Gui.Start(nullptr);
}

void StartDefaultTask(void const*) {
    DEBUG_INFO("RTOS Entrance");
    // construct app
    auto& app = PocketTrustee::App::InitOrGetApp();
    for (;;) {
        HAL_Delay(1000);
    }
    vTaskDelete(nullptr);
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
