#ifndef POCKETTRUSTEE_LL_TRNG_HPP
#define POCKETTRUSTEE_LL_TRNG_HPP

#include "app_ptt.hpp"

class TrngLowLayer final : public PocketTrustee::Device::ITrng {
public:
    TrngLowLayer(const TrngLowLayer&) = delete;
    TrngLowLayer& operator=(const TrngLowLayer&) = delete;

    static TrngLowLayer& GetInstance();

    uint32_t Gen() override;

private:
    TrngLowLayer();
};

#endif //POCKETTRUSTEE_LL_TRNG_HPP
