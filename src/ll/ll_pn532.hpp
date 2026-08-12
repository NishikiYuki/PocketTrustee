#ifndef POCKETTRUSTEE_LL_PN532_HPP
#define POCKETTRUSTEE_LL_PN532_HPP

#include "main.h"
#include "driver_pn532.hpp"

class NxpPn532LowLayer final : public INxpPn532LowLayer {
public:
    static NxpPn532LowLayer& getInstance();

    void ll_init() override;
    void enable(uint8_t flag) override;
    int write(const uint8_t* data, uint16_t len) override;
    int read(uint8_t* data, uint16_t len) override;
    void register_receive_handler(NewMessageHandler handler) override;
    void unregister_receive_handler() override;
    
private:
    NxpPn532LowLayer() = default;
};

#endif //POCKETTRUSTEE_LL_PN532_HPP
