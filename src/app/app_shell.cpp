#include "app_shell.hpp"
#include "usb_imp_acm.hpp"
#include "FreeRTOS.h"
#include "task.h"

namespace {
    constexpr uint16_t kShellStackSize = 512;
    __aligned(4) StackType_t shell_stack[kShellStackSize];
    StaticTask_t shell_task_tcb{};
    TaskHandle_t shell_task_handle{};

    __aligned(4) uint8_t package_buffer[256];

    void ShellMainThread(void*) {
        auto& device = UsbAcmDevice::GetInstance();
        device.RegisterNewMessageHandler([](uint16_t) {
            xTaskNotifyFromISR(shell_task_handle, 0, eIncrement, nullptr);
        });
        for (;;) {
            xTaskNotifyStateClear(shell_task_handle);
            ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
            auto nread = device.Read(package_buffer, sizeof(package_buffer));
            DEBUG_INFO("Received %u", nread);
        }
    }
}

ShellSocket::ShellSocket(PocketTrustee::App& app) : App(app) {
    shell_task_handle = xTaskCreateStatic(ShellMainThread, "shell",
                                          kShellStackSize, nullptr, 4,
                                          shell_stack, &shell_task_tcb);
    DEBUG_ASSERT(shell_task_handle != nullptr);
    vTaskSuspend(shell_task_handle);
}

PocketTrustee::IService& ShellSocket::GetInstance() {
    static ShellSocket instance{PocketTrustee::App::InitOrGetApp()};
    return instance;
}

void ShellSocket::Start() {
    DEBUG_ASSERT(!App.usb.HasDevice());
    App.usb.RegisterDevice(&UsbAcmDevice::GetInstance());
    CoreClock::SetSleepMode(CoreClock::CoreSleep::WfiOnly);
    vTaskResume(shell_task_handle);
    DEBUG_INFO("Shell start");
}

void ShellSocket::Stop() {
    vTaskSuspend(shell_task_handle);
    App.usb.PopCurrentDevice();
    CoreClock::SetSleepMode(CoreClock::CoreSleep::AllowStop2);
    DEBUG_INFO("Shell stop");
}
