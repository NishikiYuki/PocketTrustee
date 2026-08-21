#include "driver_rtc.hpp"
#include "main.h"

namespace {
    constexpr uint32_t kMagicNumber = 0xAABBCCDD;

    struct RtcProfile {
        uint32_t magic;
        uint32_t crash_id;
        uint32_t crc32;
    };

    uint32_t crc32_mpeg2(const uint8_t* data, uint32_t len) {
        uint32_t crc = 0xFFFFFFFF;
        for (uint32_t i = 0; i < len; ++i) {
            crc ^= static_cast<uint32_t>(data[i]) << 24;
            for (uint8_t j = 0; j < 8; ++j) {
                crc = (crc & 0x80000000U) ? (crc << 1) ^ 0x04C11DB7U : crc << 1;
            }
        }
        return crc;
    }
}

void DeviceRtc::Initialize() {
    ll_.ll_init();

    RtcProfile profile {};
    auto* p_data = reinterpret_cast<uint32_t*>(&profile);
    for (int i = 0; i < static_cast<int>(sizeof(RtcProfile) / sizeof(uint32_t)); ++i) {
        if (ll_.read_backup_ram(i, reinterpret_cast<int*>(p_data + i)) != 0) {
            Error_Handler();
        }
    }

    const uint32_t crc = crc32_mpeg2(reinterpret_cast<const uint8_t*>(&profile),
                                     sizeof(RtcProfile) - sizeof(uint32_t));
    if (profile.magic != kMagicNumber) {
        DEBUG_WARN("RTC profile not exist");
        ll_.ll_reset();
        profile = {};
        profile.crc32 = crc32_mpeg2(reinterpret_cast<const uint8_t*>(&profile),
                                    sizeof(RtcProfile) - sizeof(uint32_t));
        for (int i = 0; i < static_cast<int>(sizeof(RtcProfile) / sizeof(uint32_t)); ++i) {
            ll_.write_backup_ram(i, static_cast<int>(p_data[i]));
        }
    }
    else {
        DEBUG_WARN("RTC previous profile exist");
    }
}
