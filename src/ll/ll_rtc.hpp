#ifndef POCKETTRUSTEE_LL_RTC_HPP
#define POCKETTRUSTEE_LL_RTC_HPP

#include "app_ptt.hpp"

class RtcLowLayer final : public PocketTrustee::Device::IRtcDevice {
    RtcLowLayer();

public:
    static RtcLowLayer& GetInstance();

    RtcLowLayer(const RtcLowLayer&) = delete;
    RtcLowLayer& operator=(const RtcLowLayer&) = delete;

    void SetCount(uint64_t unix_time) override;
    uint64_t GetCount() override;
};

#endif //POCKETTRUSTEE_LL_RTC_HPP
