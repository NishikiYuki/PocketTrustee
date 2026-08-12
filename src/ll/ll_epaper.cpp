#include "ll_epaper.hpp"
#include "spi.h"

namespace {
    constexpr SPI_HandleTypeDef* kSPI = &hspi2;
    constexpr uint32_t kEpdWidth = 296;
    constexpr uint32_t kEpdHeight = 128;
    constexpr uint32_t kEpdArraySize = 4736;

    void transmit_data(const uint8_t* data, const uint16_t len) {
        DDC_GPIO_Port->BSRR = DDC_Pin;
        HAL_SPI_Transmit(kSPI, data, len, 100);
    }

    void transmit_data_dma(const uint8_t* data, const uint16_t len) {
        DDC_GPIO_Port->BSRR = DDC_Pin;
        HAL_SPI_Transmit_DMA(kSPI, data, len);
    }

    void transmit_byte_data(const uint8_t data) {
        transmit_data(&data, 1);
    }

    void transmit_cmd(const uint8_t command) {
        DDC_GPIO_Port->BRR = DDC_Pin;
        HAL_SPI_Transmit(kSPI, &command, 1, 100);
    }

    void poll_for_spi_ready() {
        uint16_t timeout = 1000;
        while (kSPI->State != HAL_SPI_STATE_READY && timeout) {
            HAL_Delay(1);
            --timeout;
        }
    }

    void poll_for_epaper_ready() {
        uint16_t timeout = 3000;
        while ((DBUSY_GPIO_Port->IDR & DBUSY_Pin) && timeout) {
            HAL_Delay(1);
            --timeout;
        }
    }

    template <typename... Args>
    void DAT(const Args&... args) {
        transmit_byte_data(args...);
    }

    template <typename... Args>
    void CMD(const Args&... args) {
        transmit_cmd(args...);
    }

    void ep_hardware_init(bool full) {
        // HWRST
        DRST_GPIO_Port->BRR = DRST_Pin;
        HAL_Delay(5);
        DRST_GPIO_Port->BSRR = DRST_Pin;
        HAL_Delay(1);
        poll_for_epaper_ready();
        // Select internal temperture sensor
        CMD(0x18);
        DAT(0x80);
        // Data entry sequence -> X decrement, Y increment
        CMD(0x11);
        DAT(0x02);
        // X Start End
        CMD(0x44);
        DAT(kEpdHeight / 8 - 1);
        DAT(0x00);
        // Y Start End
        CMD(0x45);
        DAT(0x00);
        DAT(0x00);
        DAT((kEpdWidth - 1) % 256);
        DAT((kEpdWidth - 1) / 256);
        // Reset X address
        CMD(0x4E);
        DAT(kEpdHeight / 8 - 1);
        // Reset Y address
        CMD(0x4F);
        DAT(0x00);
        DAT(0x00);
        if (!full) {
            // Display Update Control 2 -> B1
            CMD(0x22);
            DAT(0xB1);
            CMD(0x20);
            poll_for_epaper_ready();
            // Temperature Sensor Control
            CMD(0x1A);
            DAT(0x64);
            DAT(0x00);
            // Display Update Control 2 -> 91
            CMD(0x22);
            DAT(0x91);
            CMD(0x20);
            poll_for_epaper_ready();
        }
        else {
            // Driver Output control
            CMD(0x01);
            DAT((kEpdWidth - 1) % 256);
            DAT((kEpdWidth - 1) / 256);
            DAT(0x00);
            // Border Waveform
            CMD(0x3C);
            DAT(0x05);
            // Display Update Control
            CMD(0x21);
            DAT(0x00);
            DAT(0x80);
            poll_for_epaper_ready();
        }
    }

    void full_update(const uint8_t* data) {
        ep_hardware_init(true);
        CMD(0x24);
        transmit_data_dma(data, kEpdArraySize);
        poll_for_spi_ready();
        CMD(0x26);
        transmit_data_dma(data, kEpdArraySize);
        poll_for_spi_ready();
        // Same as FULL_Update
        CMD(0x22);
        DAT(0xF7);
        CMD(0x20);
        poll_for_epaper_ready();
        CMD(0x10);
        DAT(0x01);
    }

    void fast_update(const uint8_t* data) {
        ep_hardware_init(false);
        CMD(0x24);
        transmit_data_dma(data, kEpdArraySize);
        poll_for_spi_ready();
        CMD(0x26);
        transmit_data_dma(data, kEpdArraySize);
        poll_for_spi_ready();
        CMD(0x22);
        DAT(0xC7);
        CMD(0x20);
        poll_for_epaper_ready();
        CMD(0x10);
        DAT(0x01);
    }

    void partial_update(const uint8_t* data) {
        // HWRST
        DRST_GPIO_Port->BRR = DRST_Pin;
        HAL_Delay(5);
        DRST_GPIO_Port->BSRR = DRST_Pin;
        HAL_Delay(1);
        poll_for_epaper_ready();

        // Border Waveform
        CMD(0x3C);
        DAT(0x80);

        // Data entry sequence -> X decrement, Y increment
        CMD(0x11);
        DAT(0x02);
        // X Start End
        CMD(0x44);
        DAT(kEpdHeight/8-1);
        DAT(0x00);
        // Y Start End
        CMD(0x45);
        DAT(0x00);
        DAT(0x00);
        DAT((kEpdWidth-1)%256);
        DAT((kEpdWidth-1)/256);
        // Reset X address
        CMD(0x4E);
        DAT(kEpdHeight/8-1);
        // Reset Y address
        CMD(0x4F);
        DAT(0x00);
        DAT(0x00);

        CMD(0x24);
        transmit_data_dma(data, kEpdArraySize);
        poll_for_spi_ready();

        CMD(0x22);
        DAT(0xFF);
        CMD(0x20);
        poll_for_epaper_ready();
        CMD(0x10);
        DAT(0x01);
    }

    uint8_t refreash_count = 0;
}

size_t DeviceEpaper::getWidth() const {
    return kEpdWidth;
}

size_t DeviceEpaper::getHeight() const {
    return kEpdHeight;
}

void DeviceEpaper::Update(const guiv2::ReadonlyMonoBitMap& data, bool strong) {
    if (strong) {
        full_update(data.getBuffer().data());
        refreash_count = 0;
    }
    else if (refreash_count > 10) {
        fast_update(data.getBuffer().data());
        refreash_count = 0;
    }
    else {
        partial_update(data.getBuffer().data());
    }
    refreash_count++;
}

DeviceEpaper& DeviceEpaper::getInstance() {
    static DeviceEpaper instance;
    return instance;
}
