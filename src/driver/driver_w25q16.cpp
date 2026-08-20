#include "driver_w25q16.hpp"

using namespace PocketTrustee_LowLayer;

namespace {
    __aligned(8) uint8_t w25q16_prog_cache[ExternalW25Q16::kCacheSize];
    __aligned(8) uint8_t w25q16_read_cache[ExternalW25Q16::kCacheSize];
    __aligned(8) uint8_t w25q16_lookahead_cache[ExternalW25Q16::kLookaheadSize];

    int w25q16_read(const lfs_config* c, lfs_block_t block, lfs_off_t off, void* buffer, lfs_size_t size) {
        const uint32_t addr = block * c->block_size + off;
        w25q16::read_data(addr, static_cast<uint8_t*>(buffer), size);
        return LFS_ERR_OK;
    }

    int w25q16_write(const lfs_config* c, lfs_block_t block, lfs_off_t off, const void* buffer, lfs_size_t size) {
        uint32_t addr = block * c->block_size + off;
        const auto* data = static_cast<const uint8_t*>(buffer);
        while (size > 0) {
            uint16_t write_len = size;
            uint32_t page_remain = ExternalW25Q16::kPageSize - (addr % ExternalW25Q16::kPageSize);

            if (write_len > page_remain) {
                write_len = page_remain;
            }
            w25q16::page_program(addr, data, write_len);
            while (w25q16::read_busy()) {
            }
            addr += write_len;
            data += write_len;
            size -= write_len;
        }
        return LFS_ERR_OK;
    }

    int w25q16_erase(const lfs_config* c, lfs_block_t block) {
        const uint32_t addr = block * c->block_size;
        w25q16::sector_erase(addr);
        while (w25q16::read_busy()) {
        }
        return LFS_ERR_OK;
    }

    int w25q16_sync(const lfs_config* c) {
        (void)(c);
        return LFS_ERR_OK;
    }
}

lfs_config* ExternalW25Q16::getConfig() {
    static lfs_config w25q16_config = {
        .read = w25q16_read,
        .prog = w25q16_write,
        .erase = w25q16_erase,
        .sync = w25q16_sync,

        .read_size = 256,
        .prog_size = 256,
        .block_size = kBlockSize,
        .block_count = kBlockCount,
        .block_cycles = -1,
        .cache_size = kCacheSize,
        .lookahead_size = kLookaheadSize,

        .read_buffer = w25q16_read_cache,
        .prog_buffer = w25q16_prog_cache,
        .lookahead_buffer = w25q16_lookahead_cache,
    };
    return &w25q16_config;
}
