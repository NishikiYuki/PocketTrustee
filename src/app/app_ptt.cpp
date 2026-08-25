#include "app_ptt.hpp"
// Inject low layer device here
#include "ll_epaper.hpp"
#include "ll_clock.hpp"
#include "driver_w25q16.hpp"
#include "ll_rtc.hpp"
#include "ll_trng.hpp"

PocketTrustee::App& PocketTrustee::App::InitOrGetApp() {
    // Injetc gui with epaper
    static guiv2::Gui gui{DeviceEpaper::getInstance()};
    // Disk
    static auto disk = ExternalW25Q16::GetConfig();
    // Usb
    static auto& usb = DeviceUsb::GetInstance();
    // Rtc
    static auto& rtc = RtcLowLayer::GetInstance();
    // TRNG
    static auto& rng = TrngLowLayer::GetInstance();
    // Inject all device to app
    static App app{
        gui,
        disk,
        usb,
        rtc,
        rng
    };
    return app;
}

using namespace PocketTrustee;

App::App(guiv2::Gui& gui, lfs_config* disk_, DeviceUsb& usb_, Device::IRtcDevice& rtc_, Device::ITrng& rng_) :
    Gui(gui), Disk(disk_), Usb(usb_), Rtc(rtc_), Rng(rng_) {
    if (lfs_mount(&Lfs, Disk) != LFS_ERR_OK &&
        lfs_format(&Lfs, Disk) != LFS_ERR_OK &&
        lfs_mount(&Lfs, Disk) != LFS_ERR_OK) {
        DEBUG_ERROR("Fail to mount disk");
        Error_Handler();
    }
    token_promises_.reserve(8);
    DEBUG_INFO("App constructed");
}

void App::TakeActiveToken(const void* promise) {
    if (token_promises_.empty()) {
        CoreClock::SetSleepMode(CoreClock::CoreSleep::WfiOnly);
    }
    token_promises_.push_back(promise);
}

void App::DisposeActiveToken(const void* promise) {
    std::erase(token_promises_, promise);
    if (token_promises_.empty()) {
        CoreClock::SetSleepMode(CoreClock::CoreSleep::AllowStop2);
    }
}
