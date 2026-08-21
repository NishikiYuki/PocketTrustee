#ifndef POCKETTRUSTEE_APP_PTT_HPP
#define POCKETTRUSTEE_APP_PTT_HPP

#include "app_ptt_utility.hpp"
#include "ll_clock.hpp"
#include "guiv2.hpp"
#include "driver_usb.hpp"

namespace PocketTrustee {
    struct IDevice {
        virtual ~IDevice() = default;
        virtual void Initialize() = 0;
        virtual std::string_view Name() const noexcept = 0;
    };

    class App final {
        App(
            guiv2::Gui& gui,
            lfs_config* disk_,
            DeviceUsb& usb_
        ) :
        Gui(gui), disk(disk_), usb(usb_) {
            DEBUG_INFO("App constructed");
        }

    public:
        App(const App&) = delete;
        App& operator=(const App&) = delete;

        static App& InitOrGetApp();

        guiv2::Gui& Gui;
        lfs_t lfs{};
        lfs_config* disk;
        DeviceUsb& usb;
    };
}

#endif //POCKETTRUSTEE_APP_PTT_HPP
