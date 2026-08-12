#ifndef POCKETTRUSTEE_DRIVER_MULTIPLEXER_HPP
#define POCKETTRUSTEE_DRIVER_MULTIPLEXER_HPP

#include "app_ptt.hpp"

class IMultiplexerLowLayer {
public:
    enum MultiplexerChannel {
        MUX_NONE,
        MUX_CHANNEL1,
        MUX_CHANNEL2,
        MUX_CHANNEL3,
        MUX_CHANNEL4,
        MUX_BYPASS,
    };

    virtual ~IMultiplexerLowLayer() = default;

    virtual void ll_init() = 0;
    virtual void set_channel(MultiplexerChannel channel) = 0;
};

class DeviceMultiplexer final : public PocketTrustee::IDevice {
public:
    using MultiplexerChannel = IMultiplexerLowLayer::MultiplexerChannel;

    explicit DeviceMultiplexer(IMultiplexerLowLayer& ll) : ll_(ll) {}

    void Initialize() override {
        ll_.ll_init();
    }

    std::string_view Name() const noexcept override {
        return "multiplexer";
    }

    MultiplexerChannel Get();
    void Set(MultiplexerChannel channel);

private:
    IMultiplexerLowLayer& ll_;
    MultiplexerChannel channel_{};
};

#endif //POCKETTRUSTEE_DRIVER_MULTIPLEXER_HPP
