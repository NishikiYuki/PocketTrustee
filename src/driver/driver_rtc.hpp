#ifndef POCKETTRUSTEE_DRIVER_RTC_HPP
#define POCKETTRUSTEE_DRIVER_RTC_HPP

#include "app_ptt.hpp"

class IRtcLowLayer {
public:
    virtual ~IRtcLowLayer() = default;

    /**
     * @brief 配置RTC时钟源(LSE)并使能RTC。每次开机可安全调用,不会影响已有时间
     */
    virtual void ll_init() = 0;

    /**
     * @brief 重新初始化RTC核心寄存器(分频器等),会清空当前时间。
     *        仅在首次上电(备份RAM无有效档案)时调用
     */
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
