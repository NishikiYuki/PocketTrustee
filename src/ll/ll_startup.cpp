#include "main.h"
#include "FreeRTOS.h"
#include "task.h"
#include "app_ptt.hpp"
#include "ll_epaper.hpp"
#include "driver_w25q16.hpp"

extern "C" {
void StartDefaultTask(void const* argument);
void vApplicationStackOverflowHook(TaskHandle_t xTask, char* pcTaskName);
void vApplicationMallocFailedHook(void);
}

void HAL_Delay(uint32_t Delay) {
    vTaskDelay(Delay);
}

void StartDefaultTask(void const* argument) {
    DEBUG_INFO("RTOS Entrance");
    auto& app = PocketTrustee::App::GetApp();
    {
        app.disk = ExternalW25Q16::getConfig();
        DEBUG_INFO("Register Disks");
    }
    {
        app.Gui = std::make_unique<guiv2::Gui>(DeviceEpaper::getInstance());
        DEBUG_INFO("Register Gui");
    }
    {

    }
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