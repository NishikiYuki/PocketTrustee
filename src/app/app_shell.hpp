#ifndef POCKETTRUSTEE_APP_SHELL_HPP
#define POCKETTRUSTEE_APP_SHELL_HPP

#include "app_ptt.hpp"
#include "FreeRTOS.h"
#include "task.h"

class ShellSocket final {
public:
    // ShellSocket will register app usb device with acm_device
    explicit ShellSocket(PocketTrustee::App& app);
    ~ShellSocket();

    void Start(const void*);

    PocketTrustee::App& App;
private:
    void NewMessageHandler(uint16_t size);
    TaskHandle_t this_task{};
};

#endif //POCKETTRUSTEE_APP_SHELL_HPP
