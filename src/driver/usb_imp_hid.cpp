#include "usb_imp_hid.hpp"
#include "usbd_core.h"
#include "usbd_hid.h"
#include "main.h"

namespace {
    constexpr uint8_t kEndpoint_INT = 0x01 | 0x80;

    constexpr uint16_t kUsbVID = 0xFFFF;
    constexpr uint16_t kUsbPID = 0xFFFF;

    // Device Descriptor
    // As HID class required, bDeviceClass, bDeviceSubClass, bDeviceProtocol should be zero
    constexpr uint8_t device_descriptor[] = {
        USB_DEVICE_DESCRIPTOR_INIT(USB_2_0, 0x00, 0x00, 0x00, kUsbVID, kUsbPID, 0x0200, 0x01)
    };

    constexpr const uint8_t* device_descriptor_callback(uint8_t speed) { return device_descriptor; }

    constexpr uint16_t config_descriptor_size = 9 + HID_KEYBOARD_DESCRIPTOR_LEN;
    constexpr uint16_t report_descriptor_size = 63;

    // Config descriptor, Interface descriptor, Endpoint descriptor
    constexpr uint8_t config_descriptor[] = {
        USB_CONFIG_DESCRIPTOR_INIT(config_descriptor_size, 0x01, 0x01, USB_CONFIG_BUS_POWERED, 100),
        HID_KEYBOARD_DESCRIPTOR_INIT(0x00, 0x01, report_descriptor_size, kEndpoint_INT, 8, 10),
    };

    constexpr const uint8_t* config_descriptor_callback(uint8_t speed) { return config_descriptor; }

    // Quality descriptor, required as full-speed device
    constexpr uint8_t device_quality_descriptor[] = {
        0x0a, USB_DESCRIPTOR_TYPE_DEVICE_QUALIFIER, 0x00, 0x02, 0x00, 0x00, 0x00, 0x40, 0x00, 0x00,
    };

    constexpr const uint8_t* device_quality_descriptor_callback(uint8_t speed) { return device_quality_descriptor; }

    const char* string_descriptors[] = {
        (const char[]){0x09, 0x04},
        "0",
        "PocketTrustee",
        "0",
    };

    constexpr const char* string_descriptor_callback(uint8_t speed, uint8_t index) {
        if (index >= (sizeof(string_descriptors) / sizeof(char*))) {
            return nullptr;
        }
        return string_descriptors[index];
    }

    constexpr uint8_t hid_keyboard_report_desc[] = {
        0x05, 0x01, // USAGE_PAGE (Generic Desktop)
        0x09, 0x06, // USAGE (Keyboard)
        0xa1, 0x01, // COLLECTION (Application)
        0x05, 0x07, // USAGE_PAGE (Keyboard)
        0x19, 0xe0, // USAGE_MINIMUM (Keyboard LeftControl)
        0x29, 0xe7, // USAGE_MAXIMUM (Keyboard Right GUI)
        0x15, 0x00, // LOGICAL_MINIMUM (0)
        0x25, 0x01, // LOGICAL_MAXIMUM (1)
        0x75, 0x01, // REPORT_SIZE (1)
        0x95, 0x08, // REPORT_COUNT (8)
        0x81, 0x02, // INPUT (Data,Var,Abs)
        0x95, 0x01, // REPORT_COUNT (1)
        0x75, 0x08, // REPORT_SIZE (8)
        0x81, 0x03, // INPUT (Cnst,Var,Abs)
        0x95, 0x05, // REPORT_COUNT (5)
        0x75, 0x01, // REPORT_SIZE (1)
        0x05, 0x08, // USAGE_PAGE (LEDs)
        0x19, 0x01, // USAGE_MINIMUM (Num Lock)
        0x29, 0x05, // USAGE_MAXIMUM (Kana)
        0x91, 0x02, // OUTPUT (Data,Var,Abs)
        0x95, 0x01, // REPORT_COUNT (1)
        0x75, 0x03, // REPORT_SIZE (3)
        0x91, 0x03, // OUTPUT (Cnst,Var,Abs)
        0x95, 0x06, // REPORT_COUNT (6)
        0x75, 0x08, // REPORT_SIZE (8)
        0x15, 0x00, // LOGICAL_MINIMUM (0)
        0x25, 0xFF, // LOGICAL_MAXIMUM (255)
        0x05, 0x07, // USAGE_PAGE (Keyboard)
        0x19, 0x00, // USAGE_MINIMUM (Reserved (no event indicated))
        0x29, 0x65, // USAGE_MAXIMUM (Keyboard Application)
        0x81, 0x00, // INPUT (Data,Ary,Abs)
        0xc0 // END_COLLECTION
    };

    constexpr usb_descriptor hid_descriptor = {
        .device_descriptor_callback = device_descriptor_callback,
        .config_descriptor_callback = config_descriptor_callback,
        .device_quality_descriptor_callback = device_quality_descriptor_callback,
        .string_descriptor_callback = string_descriptor_callback
    };

    volatile bool hid_busy_ = true;
    volatile bool capslock_ = false;
    usbd_endpoint hid_in_ep;
    usbd_interface intf0;

    void usbd_hid_int_callback(uint8_t busid, uint8_t ep, uint32_t nbytes) {
        hid_busy_ = false;
    }

    void usbd_event_handler(uint8_t busid, uint8_t event) {
        if (event == USBD_EVENT_CONFIGURED) {
            hid_busy_ = false;
        }
    }
}

extern "C" void usbd_hid_set_report(uint8_t busid, uint8_t intf, uint8_t report_id, uint8_t report_type,
                                    uint8_t* report, uint32_t report_len) {
    (void)busid;
    (void)intf;
    (void)report_id;
    if (report_type == 0x02 && report_len >= 1) {
        capslock_ = (report[0] & 0x02) != 0;
    }
}

UsbHidDevice::UsbHidDevice() {
    hid_in_ep.ep_addr = kEndpoint_INT;
    hid_in_ep.ep_cb = usbd_hid_int_callback;
    usbd_hid_init_intf(0, &intf0, hid_keyboard_report_desc, sizeof(hid_keyboard_report_desc));
}

UsbHidDevice& UsbHidDevice::GetInstance() {
    static UsbHidDevice instance;
    return instance;
}

void UsbHidDevice::Config() {
    usbd_desc_register(0, &hid_descriptor);
    usbd_add_interface(0, &intf0);
    usbd_add_endpoint(0, &hid_in_ep);
    usbd_initialize(0, USB_BASE, usbd_event_handler);
}
