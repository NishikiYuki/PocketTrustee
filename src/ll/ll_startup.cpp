#include "main.h"
#include "FreeRTOS.h"
#include "task.h"
#include "app_ptt.hpp"
#include "app_shell.hpp"
#include "view_startup.hpp"

namespace {
    PocketTrustee::App * app_ = nullptr;
}

extern "C" {
void StartDefaultTask(void const*);
void vApplicationStackOverflowHook(TaskHandle_t xTask, char* pcTaskName);
void vApplicationMallocFailedHook(void);
}

void HAL_Delay(uint32_t Delay) {
    vTaskDelay(Delay);
}

void StartDefaultTask(void const*) {
    DEBUG_INFO("RTOS Entrance");
    // construct app
    auto& app = PocketTrustee::App::InitOrGetApp();
    app_ = &app;
    ShellSocket::GetInstance().Start();
    app.Gui.OpenWindow(std::make_unique<ViewStartup::StartupWindow>(app.Gui));
    app.Gui.Start(nullptr);
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {
    if (!app_) {
        return;
    }

    if (GPIO_Pin == FLG_CHG_Pin) {
    }
    else if (GPIO_Pin == FLG_USB_Pin) {
    }
    else {
        switch (GPIO_Pin) {
        case KUP_Pin:
            if (KUP_GPIO_Port->IDR & KUP_Pin) break;
            app_->Gui.PostEvent(guiv2::UiEvent::BasicUp);
            break;
        case KDN_Pin:
            if (KDN_GPIO_Port->IDR & KDN_Pin) break;
            app_->Gui.PostEvent(guiv2::UiEvent::BasicDown);
            break;
        case KEN_Pin:
            if (KEN_GPIO_Port->IDR & KEN_Pin) break;
            app_->Gui.PostEvent(guiv2::UiEvent::BasicEnter);
            break;
        default:
            break;
        }
    }
}
