#ifndef POCKETTRUSTEE_LL_MULTIPLEXER_HPP
#define POCKETTRUSTEE_LL_MULTIPLEXER_HPP

#include "driver_multiplexer.hpp"

class MultiplexerLowLayer final : public IMultiplexerLowLayer {
public:
    static MultiplexerLowLayer& getInstance();

    void ll_init() override;
    void set_channel(MultiplexerChannel channel) override;
private:
    MultiplexerLowLayer() = default;
};

#endif //POCKETTRUSTEE_LL_MULTIPLEXER_HPP
