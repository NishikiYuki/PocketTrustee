#ifndef POCKETTRUSTEE_LL_RTC_HPP
#define POCKETTRUSTEE_LL_RTC_HPP

#include "driver_rtc.hpp"

class RtcLowLayer final : public IRtcLowLayer {
public:
    static RtcLowLayer& getInstance();

    void ll_init() override;
    void ll_reset() override;
    int write_backup_ram(uint8_t offset, int value) override;
    int read_backup_ram(uint8_t offset, int* p_value) override;
    void set_time(PocketTrustee::DateTime dt) override;
    PocketTrustee::DateTime get_time() override;
private:
    RtcLowLayer() = default;
};

#endif //POCKETTRUSTEE_LL_RTC_HPP
