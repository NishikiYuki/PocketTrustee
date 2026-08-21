#include "app_shell.hpp"
#include "usb_imp_acm.hpp"

ShellSocket::ShellSocket(PocketTrustee::App& app) : App(app) {
    DEBUG_ASSERT(!app.usb.HasDevice());
    App.usb.RegisterDevice(&UsbAcmDevice::GetInstance());
    CoreClock::SetSleepMode(CoreClock::CoreSleep::WfiOnly);
    DEBUG_INFO("Shell constructed");
}

ShellSocket::~ShellSocket() {
    App.usb.PopCurrentDevice();
    DEBUG_INFO("Shell disposed");
}

void ShellSocket::Start(const void*) {
    this_task = xTaskGetCurrentTaskHandle();
    DEBUG_ASSERT(this_task != nullptr);
    auto& device = UsbAcmDevice::GetInstance();
    device.RegisterNewMessageHandler([this](uint16_t size) {
        NewMessageHandler(size);
    });

    uint8_t package_buffer[256];
    for (;;) {
        xTaskNotifyStateClear(this_task);
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
        auto nread = device.Read(package_buffer, sizeof(package_buffer));
        DEBUG_INFO("Received %u", nread);
    }
}

void ShellSocket::NewMessageHandler(uint16_t size) {
    xTaskNotifyFromISR(this_task, 0, eIncrement, nullptr);
}
