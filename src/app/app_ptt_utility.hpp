#ifndef POCKETTRUSTEE_APP_PTT_UTILITY_HPP
#define POCKETTRUSTEE_APP_PTT_UTILITY_HPP

#include <cstdint>
#include <string_view>

namespace PocketTrustee {
    enum class TimeZoneOffset : int32_t {
        None = 0,
        Shanghai = (+8 * 3600)
    };

    struct DateTime {
        uint16_t year{};
        uint8_t month{};
        uint8_t day{};
        uint8_t hour{};
        uint8_t minute{};
        uint8_t second{};
        TimeZoneOffset zone = TimeZoneOffset::None;
        uint64_t ToUnixTime() const;
        static DateTime FromUnixTime(uint64_t unix_time, TimeZoneOffset offset = TimeZoneOffset::None);
    };
}

#endif //POCKETTRUSTEE_APP_PTT_UTILITY_HPP
