#include "FreeRTOS.h"
#include "task.h"
#include "app_ptt.hpp"
#include "app_shell.hpp"
#include "main.h"
#include "view_home.hpp"
#include "psa/crypto.h"

namespace {
    PocketTrustee::App* app_ = nullptr;
}

extern "C" {
void StartDefaultTask(void const*);
void vApplicationStackOverflowHook(TaskHandle_t xTask, char* pcTaskName);
void vApplicationMallocFailedHook(void);
}

void vApplicationStackOverflowHook(TaskHandle_t xTask, char* pcTaskName) {
    DEBUG_ERROR("%s Stack Overflow!", pcTaskName);
}

void vApplicationMallocFailedHook() {
    DEBUG_ERROR("Malloc Failed!");
}

void HAL_Delay(uint32_t Delay) { vTaskDelay(Delay); }

void StartDefaultTask(void const*) {
    DEBUG_INFO("RTOS Entrance");
    psa_crypto_init();
    // construct app
    auto& app = PocketTrustee::App::InitOrGetApp();
    app_ = &app;
    // ShellSocket::GetInstance().Start();
    app.Gui.OpenWindow(std::make_unique<guiv2::View::Home::HomeWindow>(app.Gui, app));
    app.Gui.Start(nullptr);
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {
    if (!app_) {
        return;
    }

    // if (GPIO_Pin == FLG_CHG_Pin) {
    // }
    // else if (GPIO_Pin == FLG_USB_Pin) {
    // }
    // else {
    switch (GPIO_Pin) {
    case KUP_Pin:
        if (KUP_GPIO_Port->IDR & KUP_Pin)
            break;
        app_->Gui.PostEventFromISR(guiv2::UiEvent::BasicUp);
        break;
    case KDN_Pin:
        if (KDN_GPIO_Port->IDR & KDN_Pin)
            break;
        app_->Gui.PostEventFromISR(guiv2::UiEvent::BasicDown);
        break;
    case KEN_Pin:
        if (KEN_GPIO_Port->IDR & KEN_Pin)
            break;
        app_->Gui.PostEventFromISR(guiv2::UiEvent::BasicEnter);
        break;
    default:
        break;
    }
    // }
}
