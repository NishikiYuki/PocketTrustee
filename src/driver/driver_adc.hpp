#ifndef POCKETTRUSTEE_DRIVER_ADC_HPP
#define POCKETTRUSTEE_DRIVER_ADC_HPP

#include "app_ptt.hpp"

class IAdcLowLayer {
public:
    virtual ~IAdcLowLayer() = default;

    struct AdcAquireData {
        uint32_t vbat, lf_rssi, vref;
    };

    virtual void ll_init() = 0;
    virtual void power_down() = 0;
    virtual void scan(AdcAquireData * buffer) = 0;
};

class DeviceAdc final {
public:
    void Initialize();

    DeviceAdc(IAdcLowLayer& ll) : ll_{ll} {}

    float GetBatteryVoltage();
    float GetRSSI();

private:
    IAdcLowLayer& ll_;
};

#endif //POCKETTRUSTEE_DRIVER_ADC_HPP
