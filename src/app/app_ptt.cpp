#include "app_ptt.hpp"
#include "ll_epaper.hpp"
#include "driver_w25q16.hpp"

using namespace PocketTrustee;

App& App::InitOrGetApp() {
    static guiv2::Gui gui{DeviceEpaper::getInstance()};
    static App app{
        gui,
        ExternalW25Q16::getConfig(),
        DeviceUsb::GetInstance()
    };
    return app;
}
