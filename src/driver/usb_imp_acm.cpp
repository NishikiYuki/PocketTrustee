#include "usb_imp_acm.hpp"
#include "FreeRTOS.h"
#include "main.h"
#include "simple_buffer.hpp"
#include "task.h"
#include "usbd_cdc_acm.h"
#include "usbd_core.h"

namespace {
    constexpr uint8_t kEndpoint_IN = 0x01 | 0x80;
    constexpr uint8_t kEndpoint_OUT = 0x02;
    constexpr uint8_t kEndpoint_INT = 0x03 | 0x80;

    constexpr uint16_t kUsbVID = 0xFFFF;
    constexpr uint16_t kUsbPID = 0xFFFF;

    constexpr uint16_t kUsbConfigSize = 9 + CDC_ACM_DESCRIPTOR_LEN;

    constexpr uint16_t kCdcMaxMps = 64;
    constexpr uint16_t kRingBufferSize = 256;

    __aligned(4) uint8_t read_buffer[kCdcMaxMps];
    uint8_t rx_ringbuffer_region[kRingBufferSize + 1];
    StaticRingBuffer rx_ringbuffer{rx_ringbuffer_region, sizeof(rx_ringbuffer_region)};
    bool rx_message_overflow = false;

    std::move_only_function<void(uint16_t)> rx_new_message_handler;

    volatile bool ep_tx_busy_flag = false;

    void usbd_event_handler(uint8_t busid, uint8_t event) {
        if (event == USBD_EVENT_CONFIGURED) {
            ep_tx_busy_flag = false;
            usbd_ep_start_read(busid, kEndpoint_OUT, read_buffer, sizeof(read_buffer));
        }
    }

    void ep_in_callback(uint8_t busid, uint8_t ep, uint32_t nbytes) {
        if ((nbytes % kCdcMaxMps) == 0 && nbytes) {
            /* send zlp */
            usbd_ep_start_write(busid, kEndpoint_IN, nullptr, 0);
        }
        else {
            ep_tx_busy_flag = false;
        }
    }

    void ep_out_callback(uint8_t busid, uint8_t ep, uint32_t nbytes) {
        const bool eof = nbytes != kCdcMaxMps;
        bool message_completed = false;
        uint16_t message_size = 0;

        const UBaseType_t interruptMask = taskENTER_CRITICAL_FROM_ISR();
        if (rx_ringbuffer.write(read_buffer, nbytes, eof) != nbytes) {
            rx_message_overflow = true;
        }
        if (eof) {
            message_size = rx_ringbuffer.get_used();
            message_completed = message_size != 0 || rx_message_overflow;
        }
        taskEXIT_CRITICAL_FROM_ISR(interruptMask);

        usbd_ep_start_read(busid, kEndpoint_OUT, read_buffer, kCdcMaxMps);
        if (message_completed && rx_new_message_handler) {
            rx_new_message_handler(message_size);
        }
    }

    usbd_interface intf0{}, intf1{};
    usbd_endpoint ep_in{}, ep_out{};

    constexpr uint8_t device_descriptor[] = {
        USB_DEVICE_DESCRIPTOR_INIT(USB_2_0, 0xEF, 0x02, 0x01, kUsbVID, kUsbPID, 0x0100, 0x01)};

    constexpr const uint8_t* device_descriptor_callback(uint8_t speed) { return device_descriptor; }

    constexpr uint8_t config_descriptor[] = {
        USB_CONFIG_DESCRIPTOR_INIT(kUsbConfigSize, 0x02, 0x01, USB_CONFIG_BUS_POWERED, 100),
        CDC_ACM_DESCRIPTOR_INIT(0x00, kEndpoint_INT, kEndpoint_OUT, kEndpoint_IN, kCdcMaxMps, 0x02)};

    constexpr const uint8_t* config_descriptor_callback(uint8_t speed) { return config_descriptor; }

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

    constexpr usb_descriptor cdc_descriptor = {.device_descriptor_callback = device_descriptor_callback,
                                               .config_descriptor_callback = config_descriptor_callback,
                                               .device_quality_descriptor_callback = device_quality_descriptor_callback,
                                               .string_descriptor_callback = string_descriptor_callback};
} // namespace

UsbAcmDevice& UsbAcmDevice::GetInstance() {
    static UsbAcmDevice instance;
    return instance;
}

UsbAcmDevice::UsbAcmDevice() {
    ep_in = {
        .ep_addr = kEndpoint_IN,
        .ep_cb = ep_in_callback,
    };
    ep_out = {
        .ep_addr = kEndpoint_OUT,
        .ep_cb = ep_out_callback,
    };
}

void UsbAcmDevice::Config() {
    usbd_desc_register(0, &cdc_descriptor);
    usbd_cdc_acm_init_intf(0, &intf0);
    usbd_cdc_acm_init_intf(0, &intf1);
    usbd_add_interface(0, &intf0);
    usbd_add_interface(0, &intf1);
    usbd_add_endpoint(0, &ep_in);
    usbd_add_endpoint(0, &ep_out);
    usbd_initialize(0, USB_BASE, usbd_event_handler);
}

void UsbAcmDevice::Clear() {
    taskENTER_CRITICAL();
    rx_ringbuffer.clear();
    rx_message_overflow = false;
    taskEXIT_CRITICAL();
}

uint16_t UsbAcmDevice::Read(uint8_t* buffer, uint16_t size) {
    taskENTER_CRITICAL();
    const uint16_t read = rx_message_overflow ? 0 : rx_ringbuffer.read(buffer, size);
    if (rx_message_overflow) {
        rx_ringbuffer.clear();
        rx_message_overflow = false;
    }
    taskEXIT_CRITICAL();
    return read;
}

uint16_t UsbAcmDevice::Write(std::span<const uint8_t> buffer) {
    uint16_t timeout = 100;
    ep_tx_busy_flag = true;
    uint32_t Tickstart = HAL_GetTick();
    usbd_ep_start_write(0, kEndpoint_IN, buffer.data(), buffer.size());
    // keep synchronous
    while (ep_tx_busy_flag) {
        HAL_Delay(1);
        if (((HAL_GetTick() - Tickstart) > timeout) || (timeout == 0)) {
            return 0;
        }
    }
    return buffer.size();
}

void UsbAcmDevice::RegisterNewMessageHandler(NewMessageHandler f) { rx_new_message_handler = std::move(f); }
