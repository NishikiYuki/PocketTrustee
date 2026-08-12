#ifndef POCKETTRUSTEE_LL_ADC_HPP
#define POCKETTRUSTEE_LL_ADC_HPP

#include "driver_adc.hpp"

class AdcLowLayer final : public IAdcLowLayer {
public:
    static AdcLowLayer& getInstance();

    void ll_init() override;
    void power_down() override;
    void scan(AdcAquireData* buffer) override;
private:
    AdcLowLayer() = default;
};

#endif //POCKETTRUSTEE_LL_ADC_HPP
