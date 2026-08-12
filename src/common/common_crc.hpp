#ifndef POCKETTRUSTEE_APP_CRC_HPP
#define POCKETTRUSTEE_APP_CRC_HPP

#include <cstdint>

class CommonCrc {
public:
    virtual ~CommonCrc() = default;

    virtual uint32_t Continue(uint8_t * p_data, uint32_t size) = 0;
    virtual void Reset() = 0;
    virtual uint32_t Value() = 0;
};

class Crc32Mpeg2 : public CommonCrc {
public:
    uint32_t Continue(uint8_t* p_data, uint32_t size) override;

    void Reset() override;

    uint32_t Value() override {
        return value_;
    }

private:
    uint32_t value_ = 0xFFFFFFFF;
};

#endif //POCKETTRUSTEE_APP_CRC_HPP
