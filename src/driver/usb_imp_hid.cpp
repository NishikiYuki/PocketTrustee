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

    struct AsciiKey {
        uint8_t usage;
        bool shift;
    };

    // Printable ASCII 0x20 - 0x7E mapped to HID keyboard usage codes
    constexpr AsciiKey kAsciiKeyTable[95] = {
        {0x2C, false}, // ' '
        {0x1E, true},  // '!'
        {0x34, true},  // '"'
        {0x20, true},  // '#'
        {0x21, true},  // '$'
        {0x22, true},  // '%'
        {0x23, true},  // '&'
        {0x34, false}, // '''
        {0x26, true},  // '('
        {0x27, true},  // ')'
        {0x25, true},  // '*'
        {0x2E, true},  // '+'
        {0x36, false}, // ','
        {0x2D, false}, // '-'
        {0x37, false}, // '.'
        {0x38, false}, // '/'
        {0x27, false}, // '0'
        {0x1E, false}, // '1'
        {0x1F, false}, // '2'
        {0x20, false}, // '3'
        {0x21, false}, // '4'
        {0x22, false}, // '5'
        {0x23, false}, // '6'
        {0x24, false}, // '7'
        {0x25, false}, // '8'
        {0x26, false}, // '9'
        {0x33, true},  // ':'
        {0x33, false}, // ';'
        {0x36, true},  // '<'
        {0x2E, false}, // '='
        {0x37, true},  // '>'
        {0x38, true},  // '?'
        {0x1F, true},  // '@'
        {0x04, true},  // 'A'
        {0x05, true},  // 'B'
        {0x06, true},  // 'C'
        {0x07, true},  // 'D'
        {0x08, true},  // 'E'
        {0x09, true},  // 'F'
        {0x0A, true},  // 'G'
        {0x0B, true},  // 'H'
        {0x0C, true},  // 'I'
        {0x0D, true},  // 'J'
        {0x0E, true},  // 'K'
        {0x0F, true},  // 'L'
        {0x10, true},  // 'M'
        {0x11, true},  // 'N'
        {0x12, true},  // 'O'
        {0x13, true},  // 'P'
        {0x14, true},  // 'Q'
        {0x15, true},  // 'R'
        {0x16, true},  // 'S'
        {0x17, true},  // 'T'
        {0x18, true},  // 'U'
        {0x19, true},  // 'V'
        {0x1A, true},  // 'W'
        {0x1B, true},  // 'X'
        {0x1C, true},  // 'Y'
        {0x1D, true},  // 'Z'
        {0x2F, false}, // '['
        {0x31, false}, // '\'
        {0x30, false}, // ']'
        {0x23, true},  // '^'
        {0x2D, true},  // '_'
        {0x35, false}, // '`'
        {0x04, false}, // 'a'
        {0x05, false}, // 'b'
        {0x06, false}, // 'c'
        {0x07, false}, // 'd'
        {0x08, false}, // 'e'
        {0x09, false}, // 'f'
        {0x0A, false}, // 'g'
        {0x0B, false}, // 'h'
        {0x0C, false}, // 'i'
        {0x0D, false}, // 'j'
        {0x0E, false}, // 'k'
        {0x0F, false}, // 'l'
        {0x10, false}, // 'm'
        {0x11, false}, // 'n'
        {0x12, false}, // 'o'
        {0x13, false}, // 'p'
        {0x14, false}, // 'q'
        {0x15, false}, // 'r'
        {0x16, false}, // 's'
        {0x17, false}, // 't'
        {0x18, false}, // 'u'
        {0x19, false}, // 'v'
        {0x1A, false}, // 'w'
        {0x1B, false}, // 'x'
        {0x1C, false}, // 'y'
        {0x1D, false}, // 'z'
        {0x2F, true},  // '{'
        {0x31, true},  // '|'
        {0x30, true},  // '}'
        {0x35, true},  // '~'
    };
    static_assert(sizeof(kAsciiKeyTable) / sizeof(kAsciiKeyTable[0]) == 95);

    constexpr uint32_t kHidSendTimeoutMs = 100;

    bool hid_send_report(const uint8_t report[8]) {
        uint32_t start = HAL_GetTick();
        while (hid_busy_) {
            HAL_Delay(1);
            if (HAL_GetTick() - start > kHidSendTimeoutMs) {
                return false;
            }
        }
        hid_busy_ = true;
        usbd_ep_start_write(0, kEndpoint_INT, report, 8);
        start = HAL_GetTick();
        while (hid_busy_) {
            HAL_Delay(1);
            if (HAL_GetTick() - start > kHidSendTimeoutMs) {
                return false;
            }
        }
        return true;
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
}

UsbHidDevice& UsbHidDevice::GetInstance() {
    static UsbHidDevice instance;
    return instance;
}

void UsbHidDevice::Config() {
    usbd_desc_register(0, &hid_descriptor);
    usbd_hid_init_intf(0, &intf0, hid_keyboard_report_desc, sizeof(hid_keyboard_report_desc));
    usbd_add_interface(0, &intf0);
    usbd_add_endpoint(0, &hid_in_ep);
    usbd_initialize(0, USB_BASE, usbd_event_handler);
}

bool UsbHidDevice::TypeChar(char ch) {
    if (ch < 0x20 || ch > 0x7E) {
        return false;
    }
    const AsciiKey key = kAsciiKeyTable[static_cast<uint8_t>(ch) - 0x20];
    bool shift = key.shift;
    if (capslock_) {
        if (ch >= 'A' && ch <= 'Z') {
            shift = false;
        }
        else if (ch >= 'a' && ch <= 'z') {
            shift = true;
        }
    }
    const uint8_t press_report[8] = {static_cast<uint8_t>(shift ? 0x02 : 0x00), 0x00u, key.usage, 0x00u, 0x00u, 0x00u,
                                     0x00u, 0x00u};
    const uint8_t release_report[8] = {};
    return hid_send_report(press_report) && hid_send_report(release_report);
}

void UsbHidDevice::TypeString(std::string_view str) {
    for (char c : str) {
        TypeChar(c);
    }
}
