#ifndef POCKETTRUSTEE_USB_IMP_HID_HPP
#define POCKETTRUSTEE_USB_IMP_HID_HPP

#include "app_ptt.hpp"

class UsbHidDevice final : public PocketTrustee::Device::IUsbDevice {
    UsbHidDevice();

public:
    UsbHidDevice(const UsbHidDevice&) = delete;
    UsbHidDevice& operator=(const UsbHidDevice&) = delete;

    static UsbHidDevice& GetInstance();
    void Config() override;
    bool TypeChar(char ch);
    void TypeString(std::string_view str);
};

#endif //POCKETTRUSTEE_USB_IMP_HID_HPP
