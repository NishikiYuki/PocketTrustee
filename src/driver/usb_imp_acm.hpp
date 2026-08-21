#ifndef POCKETTRUSTEE_USB_IMP_ACM_HPP
#define POCKETTRUSTEE_USB_IMP_ACM_HPP

#include "driver_usb.hpp"
#include <cstdint>
#include <functional>
#include <span>

class UsbAcmDevice final : public IUsbDevice {
    UsbAcmDevice();
public:
    UsbAcmDevice(const UsbAcmDevice&) = delete;
    UsbAcmDevice& operator=(const UsbAcmDevice&) = delete;

    static UsbAcmDevice& GetInstance();

    void Config() override;

    using NewMessageHandler = std::move_only_function<void (uint16_t)>;

    void RegisterNewMessageHandler(NewMessageHandler f);
    uint16_t Read(uint8_t* buffer, uint16_t size);
    uint16_t Write(std::span<const uint8_t> buffer);
};

#endif //POCKETTRUSTEE_USB_IMP_ACM_HPP
