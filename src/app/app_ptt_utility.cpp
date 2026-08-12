#include "app_ptt_utility.hpp"

using namespace PocketTrustee;

namespace {
    constexpr int64_t days_from_civil(int64_t y, unsigned m, unsigned d) {
        y -= m <= 2;
        const int64_t era = (y >= 0 ? y : y - 399) / 400;
        const unsigned yoe = static_cast<unsigned>(y - era * 400);
        const unsigned doy = (153 * (m + (m > 2 ? -3 : 9)) + 2) / 5 + d - 1;
        const unsigned doe = yoe * 365 + yoe / 4 - yoe / 100 + doy;
        return era * 146097 + static_cast<int64_t>(doe) - 719468;
    }

    constexpr void civil_from_days(int64_t z, int64_t& y, unsigned& m, unsigned& d) {
        z += 719468;
        const int64_t era = (z >= 0 ? z : z - 146096) / 146097;
        const unsigned doe = static_cast<unsigned>(z - era * 146097);
        const unsigned yoe = (doe - doe / 1460 + doe / 36524 - doe / 146096) / 365;
        const int64_t y_ = static_cast<int64_t>(yoe) + era * 400;
        const unsigned doy = doe - (365 * yoe + yoe / 4 - yoe / 100);
        const unsigned mp = (5 * doy + 2) / 153;
        d = doy - (153 * mp + 2) / 5 + 1;
        m = mp < 10 ? mp + 3 : mp - 9;
        y = y_ + (m <= 2);
    }
}

uint64_t DateTime::ToUnixTime() const {
    const int64_t days = days_from_civil(year, month, day);
    const int64_t local = days * 86400 + static_cast<int64_t>(hour) * 3600
                        + static_cast<int64_t>(minute) * 60 + second;
    const int64_t utc = local - static_cast<int64_t>(zone);
    return utc < 0 ? 0 : static_cast<uint64_t>(utc);
}

DateTime DateTime::FromUnixTime(uint64_t unix_time, TimeZoneOffset offset) {
    const int64_t local = static_cast<int64_t>(unix_time) + static_cast<int64_t>(offset);
    DateTime dt{};

    int64_t days = local / 86400;
    int64_t secs = local % 86400;
    if (secs < 0) {
        secs += 86400;
        --days;
    }
    dt.hour = static_cast<uint8_t>(secs / 3600);
    dt.minute = static_cast<uint8_t>(secs % 3600 / 60);
    dt.second = static_cast<uint8_t>(secs % 60);

    int64_t y;
    unsigned m, d;
    civil_from_days(days, y, m, d);

    dt.year = static_cast<uint16_t>(y);
    dt.month = static_cast<uint8_t>(m);
    dt.day = static_cast<uint8_t>(d);
    dt.zone = offset;

    return dt;
}
