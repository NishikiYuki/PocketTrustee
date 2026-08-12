#ifndef POCKETTRUSTEE_DRIVER_PN532_HPP
#define POCKETTRUSTEE_DRIVER_PN532_HPP

#include "app_ptt.hpp"
#include <functional>

class INxpPn532LowLayer {
public:
    using NewMessageHandler = std::move_only_function<void (INxpPn532LowLayer*, uint16_t)>;

    virtual ~INxpPn532LowLayer() = default;

    virtual void ll_init() = 0;
    virtual void enable(uint8_t flag) = 0;
    virtual int write(const uint8_t* data, uint16_t len) = 0;
    virtual int read(uint8_t* data, uint16_t len) = 0;
    virtual void register_receive_handler(NewMessageHandler handler) = 0;
    virtual void unregister_receive_handler() = 0;
};

class DeviceNxpPn532 final : public PocketTrustee::IDevice {
public:
    void Initialize() override;

    std::string_view Name() const noexcept override {
        return "nxp_pn532";
    }

    explicit DeviceNxpPn532(INxpPn532LowLayer& port) : ll_(port) {
    }

    using ReceiveHandler = std::move_only_function<void (DeviceNxpPn532*, uint16_t)>;

    void TransparentSend(const uint8_t* data, uint16_t len) const {
        ll_.write(data, len);
    }

    void RegisterTransparentReceive(ReceiveHandler handler) {
        ll_.register_receive_handler([&](INxpPn532LowLayer*, uint16_t size) {
            handler(this, size);
        });
    }

    void UnregisterTransparentReceive() {
        ll_.unregister_receive_handler();
    }

    void Reset();

private:
    INxpPn532LowLayer& ll_;
};

#endif //POCKETTRUSTEE_DRIVER_PN532_HPP
