#include "driver_usb.hpp"
#include "usbd_core.h"

DeviceUsb& DeviceUsb::GetInstance() {
    static DeviceUsb device;
    return device;
}

void DeviceUsb::PopCurrentDevice() {
    if (!HasDevice()) {
        return;
    }
    usbd_deinitialize(0);
    device = nullptr;
}

void DeviceUsb::RegisterDevice(IUsbDevice * d) {
    if (HasDevice()) {
        return;
    }
    device = d;
    device->Config();
}
