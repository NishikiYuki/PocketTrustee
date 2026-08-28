#ifndef POCKETTRUSTEE_APP_CRC_HPP
#define POCKETTRUSTEE_APP_CRC_HPP

#include <cstdint>

class Crc32Mpeg2 {
public:
    uint32_t Continue(const uint8_t* p_data, uint32_t size);
    void Reset();
    uint32_t Value() const { return value_; }

private:
    uint32_t value_ = 0xFFFFFFFF;
};

#endif //POCKETTRUSTEE_APP_CRC_HPP
