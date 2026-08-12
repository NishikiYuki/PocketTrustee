#include "driver_adc.hpp"

void DeviceAdc::Initialize() {
    ll_.power_down();
}

float DeviceAdc::GetBatteryVoltage() {
    IAdcLowLayer::AdcAquireData aquire {};
    ll_.ll_init();
    ll_.scan(&aquire);
    ll_.power_down();
    return static_cast<float>(aquire.vbat) / static_cast<float>(aquire.vref) * 2.4f;
}

float DeviceAdc::GetRSSI() {
    IAdcLowLayer::AdcAquireData aquire {};
    ll_.ll_init();
    ll_.scan(&aquire);
    ll_.power_down();
    return static_cast<float>(aquire.lf_rssi) / static_cast<float>(aquire.vref) * 1.2f;
}
