#include "common_crc.hpp"
#include "main.h"

// Hardware implement on STM32
#ifdef STM32L4
#ifdef CRC

static void crc32mpeg2_hardware_ready(uint32_t initVal) {
    LL_CRC_SetInitialData(CRC, initVal);
    LL_CRC_SetInputDataReverseMode(CRC, LL_CRC_INDATA_REVERSE_NONE);
    LL_CRC_SetOutputDataReverseMode(CRC, LL_CRC_OUTDATA_REVERSE_NONE);
    LL_CRC_SetPolynomialSize(CRC, LL_CRC_POLYLENGTH_32B);
    LL_CRC_SetPolynomialCoef(CRC, 0x04C11DB7);
    LL_CRC_ResetCRCCalculationUnit(CRC);
}

void Crc32Mpeg2::Reset() {
    value_ = 0xFFFFFFFF;
}

uint32_t Crc32Mpeg2::Continue(uint8_t* p_data, uint32_t size) {
    crc32mpeg2_hardware_ready(value_);
    for (uint32_t i = 0; i < size; i++) {
        LL_CRC_FeedData8(CRC, p_data[i]);
    }
    return value_ = LL_CRC_ReadData32(CRC);
}

#endif
#endif
