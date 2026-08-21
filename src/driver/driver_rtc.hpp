#ifndef POCKETTRUSTEE_DRIVER_RTC_HPP
#define POCKETTRUSTEE_DRIVER_RTC_HPP

#include "app_ptt.hpp"

class IRtcLowLayer {
public:
    virtual ~IRtcLowLayer() = default;

    virtual void ll_init() = 0;
    virtual void ll_reset() = 0;
    virtual int write_backup_ram(uint8_t offset, int value) = 0;
    virtual int read_backup_ram(uint8_t offset, int * p_value) = 0;
    virtual void set_time(PocketTrustee::DateTime dt) = 0;
    virtual PocketTrustee::DateTime get_time() = 0;
};

class DeviceRtc final : public PocketTrustee::IDevice {
public:
    void Initialize() override;

    std::string_view Name() const noexcept override {
        return "rtc";
    }

    explicit DeviceRtc(IRtcLowLayer& ll) : ll_{ll} {}

    PocketTrustee::DateTime GetDateTime() {
        return ll_.get_time();
    }

    void SetDateTime(const PocketTrustee::DateTime& dt) {
        ll_.set_time(dt);
    }

private:
    IRtcLowLayer& ll_;
};

#endif //POCKETTRUSTEE_DRIVER_RTC_HPP
