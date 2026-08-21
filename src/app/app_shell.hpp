#ifndef POCKETTRUSTEE_APP_SHELL_HPP
#define POCKETTRUSTEE_APP_SHELL_HPP

#include "app_ptt.hpp"

class ShellSocket final {
public:
    // ShellSocket will register app usb device with acm_device
    explicit ShellSocket(PocketTrustee::App& app);
    ~ShellSocket();

    PocketTrustee::App& App;
};

#endif //POCKETTRUSTEE_APP_SHELL_HPP
