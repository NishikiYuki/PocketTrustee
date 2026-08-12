#ifndef POCKETTRUSTEE_LL_EPAPER_HPP
#define POCKETTRUSTEE_LL_EPAPER_HPP

#include "guiv2.hpp"

class DeviceEpaper final : public guiv2::IDisplay {
public:
    size_t getWidth() const override;
    size_t getHeight() const override;
    void Update(const guiv2::ReadonlyMonoBitMap& data, bool strong) override;

    static DeviceEpaper& getInstance();
};

#endif //POCKETTRUSTEE_LL_EPAPER_HPP
