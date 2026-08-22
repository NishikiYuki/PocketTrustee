#include "driver_w25q16.hpp"
#include "spi.h"

namespace {
    constexpr SPI_HandleTypeDef* kSPI = &hspi1;
    constexpr uint32_t kW25q16ReadyTimeout = 2000;

    void poll_for_spi_ready() {
        uint16_t timeout = 1000;
        while (kSPI->State != HAL_SPI_STATE_READY && timeout) {
            // TODO: find out a way that only force task reschedule while keeping timeout function work
            HAL_Delay(1);
            --timeout;
        }
    }

    void transmit_cmd(const uint8_t* data, uint16_t len) { HAL_SPI_Transmit(&hspi1, data, len, 100); }

    void transmit_data(const uint8_t* data, const uint16_t len) {
        if (len < 16)
            HAL_SPI_Transmit(&hspi1, data, len, 100);
        else {
            HAL_SPI_Transmit_DMA(&hspi1, data, len);
            poll_for_spi_ready();
        }
    }

    void receive_data(uint8_t* rx_data, const uint16_t len) {
        if (len < 16)
            HAL_SPI_Receive(&hspi1, rx_data, len, 100);
        else {
            HAL_SPI_Receive_DMA(&hspi1, rx_data, len);
            poll_for_spi_ready();
        }
    }

    void write_enable() {
        constexpr uint8_t cmd = 0x06;
        FCS_GPIO_Port->BRR = FCS_Pin;
        transmit_cmd(&cmd, 1);
        FCS_GPIO_Port->BSRR = FCS_Pin;
    }
} // namespace

namespace PocketTrustee_LowLayer::w25q16 {
    void read_data(const uint32_t addr, uint8_t* rx_buffer, const uint16_t len) {
        const uint8_t cmd[4]{
            0x03,
            static_cast<uint8_t>(addr >> 16),
            static_cast<uint8_t>(addr >> 8),
            static_cast<uint8_t>(addr),
        };
        FCS_GPIO_Port->BRR = FCS_Pin;
        transmit_cmd(cmd, 4);
        receive_data(rx_buffer, len);
        FCS_GPIO_Port->BSRR = FCS_Pin;
    }

    void page_program(const uint32_t addr, const uint8_t* data, const uint16_t len) {
        write_enable();
        const uint8_t cmd[4]{
            0x02,
            static_cast<uint8_t>(addr >> 16),
            static_cast<uint8_t>(addr >> 8),
            static_cast<uint8_t>(addr),
        };
        FCS_GPIO_Port->BRR = FCS_Pin;
        transmit_cmd(cmd, 4);
        transmit_data(data, len);
        FCS_GPIO_Port->BSRR = FCS_Pin;
    }

    void sector_erase(const uint32_t addr) {
        write_enable();
        const uint8_t cmd[4]{
            0x20,
            static_cast<uint8_t>(addr >> 16),
            static_cast<uint8_t>(addr >> 8),
            static_cast<uint8_t>(addr),
        };
        FCS_GPIO_Port->BRR = FCS_Pin;
        transmit_cmd(cmd, 4);
        FCS_GPIO_Port->BSRR = FCS_Pin;
    }

    void chip_erase() {
        constexpr uint8_t cmd = 0x60;
        write_enable();
        FCS_GPIO_Port->BRR = FCS_Pin;
        transmit_cmd(&cmd, 1);
        FCS_GPIO_Port->BSRR = FCS_Pin;
    }

    bool read_busy() {
        constexpr uint8_t cmd = 0x05;
        uint8_t status = 0;
        FCS_GPIO_Port->BRR = FCS_Pin;
        transmit_cmd(&cmd, 1);
        receive_data(&status, 1);
        FCS_GPIO_Port->BSRR = FCS_Pin;
        if (status & 1) {
            return true;
        }
        return false;
    }

    bool wait_ready() {
        const uint32_t start = HAL_GetTick();
        while (read_busy()) {
            if ((HAL_GetTick() - start) >= kW25q16ReadyTimeout) {
                return false;
            }
            HAL_Delay(1);
        }
        return true;
    }
} // namespace PocketTrustee_LowLayer::w25q16
