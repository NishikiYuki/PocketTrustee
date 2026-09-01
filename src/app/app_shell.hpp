#ifndef POCKETTRUSTEE_APP_SHELL_HPP
#define POCKETTRUSTEE_APP_SHELL_HPP

#include "app_ptt.hpp"
#include "usb_imp_acm.hpp"

namespace PocketTrustee {
    class ShellSocket final {
        explicit ShellSocket(App& app, UsbAcmDevice& port) : HostApp(app), Port(port) {
        };

    public:
        ShellSocket(const ShellSocket&) = delete;
        ShellSocket& operator=(const ShellSocket&) = delete;

        static ShellSocket& GetInstance();

        void Start();
        void Stop();
        void WaitForResponse(uint32_t timeout);

        static bool SendCommandError(ShellSocket& shell, int error);

        __aligned(4) uint8_t RxBuffer[256]{};
        __aligned(4) uint8_t TxBuffer[256]{};

        App& HostApp;
        UsbAcmDevice& Port;
    };




}

#endif //POCKETTRUSTEE_APP_SHELL_HPP
