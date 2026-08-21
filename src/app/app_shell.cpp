#include "app_shell.hpp"
#include "usb_imp_acm.hpp"

ShellSocket::ShellSocket(PocketTrustee::App& app) : App(app) {
    DEBUG_ASSERT(!app.usb.HasDevice());
    App.usb.RegisterDevice(&UsbAcmDevice::GetInstance());
}

ShellSocket::~ShellSocket() {
    App.usb.PopCurrentDevice();
}
