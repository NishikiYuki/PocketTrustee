#ifndef POCKETTRUSTEE_USB_IMP_HID_HPP
#define POCKETTRUSTEE_USB_IMP_HID_HPP

#include "driver_usb.hpp"
#include <cstdint>

class UsbHidDevice final : public IUsbDevice {
    UsbHidDevice();

public:
    UsbHidDevice(const UsbHidDevice&) = delete;
    UsbHidDevice& operator=(const UsbHidDevice&) = delete;

    static UsbHidDevice& GetInstance();

    void Config() override;


};

#endif //POCKETTRUSTEE_USB_IMP_HID_HPP
