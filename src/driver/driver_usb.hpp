#ifndef POCKETTRUSTEE_DRIVER_USB_HPP
#define POCKETTRUSTEE_DRIVER_USB_HPP

class IUsbDevice;

class DeviceUsb final {
    DeviceUsb() = default;

public:
    DeviceUsb(const DeviceUsb&) = delete;
    DeviceUsb& operator=(const DeviceUsb&) = delete;

    static DeviceUsb& GetInstance();
    void PopCurrentDevice();
    bool HasDevice() { return device != nullptr; }
    void RegisterDevice(IUsbDevice * d);

private:
    IUsbDevice * device;
};

class IUsbDevice {
public:
    virtual ~IUsbDevice() = default;
    virtual void Config() = 0;
};

#endif //POCKETTRUSTEE_DRIVER_USB_HPP
