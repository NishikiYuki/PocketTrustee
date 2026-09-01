#include "app_shell.hpp"
#include "FreeRTOS.h"
#include "command_common.hpp"
#include "task.h"
#include "usb_imp_acm.hpp"

using namespace PocketTrustee;

namespace {
    constexpr uint16_t kShellStackSize = 512;
    __aligned(4) StackType_t shell_stack[kShellStackSize];
    StaticTask_t shell_task_tcb{};
    TaskHandle_t shell_task_handle{};

    // Command Instance
    CommandDeviceInfo deviceInfo;
    CommandLs ls;
    CommandMkdir mkdir;
    CommandRm rm;
    CommandMv mv;
    CommandWrite write;
    CommandRead read;
    CommandFormat format;

    constexpr ICommand* CommandList[] = {
        // Privilege level basic
        &deviceInfo,
        // Privilege level lfs
        &ls,
        &mkdir,
        &rm,
        &mv,
        &write,
        &read,
        &format,
    };

    consteval size_t GetCommandOffset(ICommand* cmd) {
        for (size_t i = 0; i < sizeof(CommandList) / sizeof(ICommand*); i++) {
            if (cmd == CommandList[i]) {
                return i;
            }
        }
        return 0;
    }

    enum class PrivilegeLevel : uint8_t {
        Basic = GetCommandOffset(&deviceInfo),
        Lfs = GetCommandOffset(&format),
    };

    std::span<ICommand* const> GetPrivilegedCommands(PrivilegeLevel level) {
        return std::span<ICommand* const>{CommandList}.first(static_cast<size_t>(level));
    }

    void ShellMessageHandler(uint16_t) {
        BaseType_t higherPriorityTaskWoken = pdFALSE;
        if (shell_task_handle) {
            vTaskNotifyGiveFromISR(shell_task_handle, &higherPriorityTaskWoken);
        }
        portYIELD_FROM_ISR(higherPriorityTaskWoken);
    }

    void ShellMainThread(void*) {
        auto& shell = ShellSocket::GetInstance();
        auto& port = UsbAcmDevice::GetInstance();
        PrivilegeLevel level = PrivilegeLevel::Basic;
        for (;;) {
            ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
            const auto nread = port.Read(shell.RxBuffer, sizeof(shell.RxBuffer));
            DEBUG_INFO("Received %u", nread);
            if (nread == 0) {
                continue;
            }
            // parse req
            ZCBOR_STATE_D(state, 2, shell.RxBuffer, nread, 1, 0);
            zcbor_string req{};
            bool success = zcbor_list_start_decode(state);
            success = success && zcbor_tstr_decode(state, &req);
            if (!success) {
                DEBUG_INFO("parse failed");
                continue;
            }
            std::string_view req_str{reinterpret_cast<const char*>(req.value), req.len};
            // construct request
            ICommand::Request request = {
                .state = state,
                .shell = ShellSocket::GetInstance(),
            };
            ICommand* candidate = nullptr;
            for (auto& cmd : GetPrivilegedCommands(level)) {
                if (cmd->Name() == req_str) {
                    candidate = cmd;
                    break;
                }
            }
            if (!candidate) {
                continue;
            }
            DEBUG_INFO("Invoke %s", candidate->Name().data());
            candidate->Invoke(request);
        }
    }
} // namespace

ShellSocket& ShellSocket::GetInstance() {
    static ShellSocket instance{PocketTrustee::App::InitOrGetApp(), UsbAcmDevice::GetInstance()};
    return instance;
}

void ShellSocket::Start() {
    if (!shell_task_handle) {
        shell_task_handle =
            xTaskCreateStatic(
                ShellMainThread, "shell", kShellStackSize,
                nullptr, 4, shell_stack, &shell_task_tcb);
    }
    else {
        vTaskResume(shell_task_handle);
    }
    auto& port = UsbAcmDevice::GetInstance();
    port.RegisterNewMessageHandler(ShellMessageHandler);
    port.Clear();
    HostApp.RegisterUsbDevice(&port);
    DEBUG_INFO("Shell start");
}

void ShellSocket::Stop() {
    if (shell_task_handle) {
        vTaskSuspend(shell_task_handle);
    }
    HostApp.PopUsbDevice();
    UsbAcmDevice::GetInstance().Clear();
    DEBUG_INFO("Shell stop");
}

void ShellSocket::WaitForResponse(uint32_t timeout) { ulTaskNotifyTake(pdTRUE, timeout); }

bool ShellSocket::SendCommandError(ShellSocket& shell, int error) {
    ZCBOR_STATE_E(state, 1, shell.TxBuffer, sizeof(shell.TxBuffer), 2);
    if (!zcbor_list_start_encode(state, 2) || !zcbor_tstr_put_lit(state, "error") ||
        !zcbor_int32_put(state, error) || !zcbor_list_end_encode(state, 2)) {
        return false;
    }
    return shell.Port.Write({shell.TxBuffer, static_cast<size_t>(state->payload - shell.TxBuffer)}) ==
        static_cast<size_t>(state->payload - shell.TxBuffer);
}
