#include "ll_adc.hpp"
#include "adc.h"

namespace {
    uint16_t adc_buffer[3];
}

AdcLowLayer& AdcLowLayer::getInstance() {
    static AdcLowLayer instance;
    return instance;
}

void AdcLowLayer::ll_init() {
    MX_ADC1_Init();
    HAL_ADCEx_Calibration_Start(&hadc1, ADC_SINGLE_ENDED);
}

void AdcLowLayer::power_down() {
    HAL_ADC_Stop_DMA(&hadc1);
    HAL_ADCEx_EnterADCDeepPowerDownMode(&hadc1);
}

void AdcLowLayer::scan(AdcAquireData* buffer) {
    HAL_ADC_Start_DMA(&hadc1, reinterpret_cast<uint32_t*>(adc_buffer), 3);
    HAL_ADC_PollForConversion(&hadc1, 1000);
    buffer->vbat = adc_buffer[0];
    buffer->lf_rssi = adc_buffer[1];
    buffer->vref = adc_buffer[2];
}
