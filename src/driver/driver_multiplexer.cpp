#include "driver_multiplexer.hpp"

DeviceMultiplexer::MultiplexerChannel DeviceMultiplexer::Get() {
    return channel_;
}

void DeviceMultiplexer::Set(MultiplexerChannel channel) {
    channel_ = channel;
    ll_.set_channel(channel_);
}
