#include "driver_rtc.hpp"
#include "main.h"

namespace {
    /**
     * 备份RAM中的设备档案:用于判断RTC是否首次上电(或电池断电导致数据丢失)。
     * 档案有效 -> 说明RTC时间曾被校准过,不应重新初始化RTC寄存器
     * 档案无效 -> 首次上电,初始化RTC并写入新档案
     */
    struct RtcProfile {
        uint32_t uuid1, uuid2, uuid3;
        uint32_t crash_id;
        uint32_t crc32;
    };
    static_assert(sizeof(RtcProfile) == sizeof(uint32_t) * 5);

    // CRC-32/MPEG-2:与CRC外设配置一致(多项式0x04C11DB7,初值0xFFFFFFFF,不反转,无最终异或)
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
    // 仅配置时钟并使能RTC,不影响已有时间
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
    if (profile.uuid1 != DEVICE_UID1 || profile.uuid2 != DEVICE_UID2 ||
        profile.uuid3 != DEVICE_UID3 || profile.crc32 != crc) {
        // 首次上电或电池断电:初始化RTC寄存器并写入档案
        DEBUG_WARN("profile not exist");
        ll_.ll_reset();
        profile = {};
        profile.uuid1 = DEVICE_UID1;
        profile.uuid2 = DEVICE_UID2;
        profile.uuid3 = DEVICE_UID3;
        profile.crc32 = crc32_mpeg2(reinterpret_cast<const uint8_t*>(&profile),
                                    sizeof(RtcProfile) - sizeof(uint32_t));
        for (int i = 0; i < static_cast<int>(sizeof(RtcProfile) / sizeof(uint32_t)); ++i) {
            ll_.write_backup_ram(i, static_cast<int>(p_data[i]));
        }
    }
    else {
        DEBUG_WARN("previous profile exist");
    }
}
