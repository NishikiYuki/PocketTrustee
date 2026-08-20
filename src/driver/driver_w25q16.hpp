#ifndef POCKETTRUSTEE_DRIVER_W25Q16_HPP
#define POCKETTRUSTEE_DRIVER_W25Q16_HPP

#include "app_ptt.hpp"

class ExternalW25Q16 final {
public:
    ExternalW25Q16() = delete;
    ExternalW25Q16(const ExternalW25Q16&) = delete;
    ExternalW25Q16& operator=(const ExternalW25Q16&) = delete;

    static constexpr uint32_t kCacheSize = 256;
    static constexpr uint32_t kLookaheadSize = 64;

    static constexpr uint32_t kPageSize = 256;
    static constexpr uint32_t kBlockSize = 4096;
    static constexpr uint32_t kBlockCount = 512;

    static lfs_config * getConfig();
};

namespace PocketTrustee_LowLayer::w25q16 {
    void read_data(const uint32_t addr, uint8_t* rx_buffer, const uint16_t len);
    void page_program(const uint32_t addr, const uint8_t* data, const uint16_t len);
    void sector_erase(const uint32_t addr);
    void chip_erase();
    bool read_busy();
}

#endif //POCKETTRUSTEE_DRIVER_W25Q16_HPP
