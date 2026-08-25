#include "ll_trng.hpp"
#include "rng.h"

TrngLowLayer::TrngLowLayer() {
    LL_RNG_Enable(RNG);
}

TrngLowLayer& TrngLowLayer::GetInstance() {
    static TrngLowLayer instance;
    return instance;
}

uint32_t TrngLowLayer::Gen() {
    return LL_RNG_ReadRandData32(RNG);
}
