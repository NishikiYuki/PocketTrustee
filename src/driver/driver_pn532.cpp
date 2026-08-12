#include "driver_pn532.hpp"
#include <bits/this_thread_sleep.h>

void DeviceNxpPn532::Initialize() {
    ll_.ll_init();
}

void DeviceNxpPn532::Reset() {
    ll_.enable(0);
    std::this_thread::sleep_for(std::chrono::milliseconds(5));
    ll_.enable(1);
    std::this_thread::sleep_for(std::chrono::milliseconds(5));
}
