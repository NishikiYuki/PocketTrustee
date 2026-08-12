#include "ll_pn532.hpp"
#include "../common/simple_buffer.hpp"
#include "usart.h"

namespace {
    uint8_t uart_region[128], ring_region[128];
    StaticRingBuffer buffer{ring_region, sizeof(ring_region)};
    NxpPn532LowLayer::NewMessageHandler extern_handler_;
}

NxpPn532LowLayer& NxpPn532LowLayer::getInstance() {
    static NxpPn532LowLayer instance{};
    return instance;
}

void NxpPn532LowLayer::ll_init() {
    HAL_UART_RegisterRxEventCallback(&huart3, [](UART_HandleTypeDef* uart, uint16_t size) {
        buffer.write(uart_region, size, size < sizeof(uart_region));
        if (extern_handler_) extern_handler_(&getInstance(), size);
        HAL_UARTEx_ReceiveToIdle_DMA(&huart3, uart_region, sizeof(uart_region));
    });
    HAL_UARTEx_ReceiveToIdle_DMA(&huart3, uart_region, sizeof(uart_region));
}

void NxpPn532LowLayer::enable(uint8_t flag) {
    if (flag) {
        PN_PD_GPIO_Port->BSRR = PN_PD_Pin;
    }
    else {
        PN_PD_GPIO_Port->BRR = PN_PD_Pin;
    }
}

int NxpPn532LowLayer::write(const uint8_t* data, uint16_t len) {
    uint8_t timeout = 50;
    while (huart3.gState != HAL_UART_STATE_READY && timeout--) {
        HAL_Delay(1);
    }
    HAL_UART_Transmit_DMA(&huart3, data, len);
    return len;
}

int NxpPn532LowLayer::read(uint8_t* data, uint16_t len) {
    return buffer.read(data, len);
}

void NxpPn532LowLayer::register_receive_handler(NewMessageHandler handler) {
    extern_handler_ = std::move(handler);
}

void NxpPn532LowLayer::unregister_receive_handler() {
    extern_handler_ = nullptr;
}
