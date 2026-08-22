#include "app_shell.hpp"
#include "FreeRTOS.h"
#include "command_common.hpp"
#include "task.h"
#include "usb_imp_acm.hpp"

namespace {
    constexpr uint16_t kShellStackSize = 512;
    __aligned(4) StackType_t shell_stack[kShellStackSize];
    StaticTask_t shell_task_tcb{};
    TaskHandle_t shell_task_handle{};

    CommandDeviceInfo deviceInfo;
    CommandLs ls;
    CommandMkdir mkdir;
    CommandRm rm;
    CommandMv mv;
    CommandWrite write;
    CommandRead read;
    ICommand* commandList[] = {
        &deviceInfo, &ls, &mkdir, &rm, &mv, &write, &read,
    };

    ICommand* currentCommand = nullptr;

    void ShellMainThread(void*) {
        auto& shell = ShellSocket::GetInstance();
        auto& port = UsbAcmDevice::GetInstance();
        port.RegisterNewMessageHandler([](uint16_t) { vTaskNotifyGiveFromISR(shell_task_handle, nullptr); });
        for (;;) {
            port.Clear();
            xTaskNotifyStateClear(shell_task_handle);
            ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
            auto nread = port.Read(shell.RxBuffer, sizeof(shell.RxBuffer));
            DEBUG_INFO("Received %u", nread);
            if (!currentCommand) {
                ZCBOR_STATE_D(state, 2, shell.RxBuffer, nread, 1, 0);
                zcbor_string req{};
                bool success = zcbor_list_start_decode(state);
                success = success && zcbor_tstr_decode(state, &req);
                if (!success) {
                    DEBUG_INFO("parse failed");
                    continue;
                }
                std::string_view req_str{reinterpret_cast<const char*>(req.value), req.len};
                ICommand::Request request = {
                    .state = state,
                    .shell = ShellSocket::GetInstance(),
                };
                for (auto& command : commandList) {
                    if (req_str == command->Name()) {
                        currentCommand = command;
                        break;
                    }
                }
                if (currentCommand) {
                    DEBUG_INFO("Invoke %s", currentCommand->Name().data());
                    currentCommand->Invoke(request);
                }
            }
            currentCommand = nullptr;
        }
    }
}

ShellSocket& ShellSocket::GetInstance() {
    static ShellSocket instance{PocketTrustee::App::InitOrGetApp(), UsbAcmDevice::GetInstance()};
    return instance;
}

void ShellSocket::Start() {
    DEBUG_ASSERT(!App.usb.HasDevice());
    App.usb.RegisterDevice(&UsbAcmDevice::GetInstance());
    CoreClock::SetSleepMode(CoreClock::CoreSleep::WfiOnly);
    shell_task_handle =
        xTaskCreateStatic(ShellMainThread, "shell", kShellStackSize, nullptr, 4, shell_stack, &shell_task_tcb);
    DEBUG_ASSERT(shell_task_handle != nullptr);
    DEBUG_INFO("Shell start");
}

void ShellSocket::Stop() {
    vTaskDelete(shell_task_handle);
    App.usb.PopCurrentDevice();
    CoreClock::SetSleepMode(CoreClock::CoreSleep::AllowStop2);
    DEBUG_INFO("Shell stop");
}

void ShellSocket::WaitForResponse(uint32_t timeout) {
    xTaskNotifyStateClear(shell_task_handle);
    ulTaskNotifyTake(pdTRUE, timeout);
}
