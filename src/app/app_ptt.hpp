#ifndef POCKETTRUSTEE_APP_PTT_HPP
#define POCKETTRUSTEE_APP_PTT_HPP

#include "app_ptt_utility.hpp"
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
            token_promises_.reserve(8);
            DEBUG_INFO("App constructed");
        }

    public:
        App(const App&) = delete;
        App& operator=(const App&) = delete;

        static App& InitOrGetApp();

        // To better manager the sleep behaviour, tasks now can take or dispose active token
        // Once no task have token, system is allowed to enter sleep mode
        void TakeActiveToken(const void * promise);
        void DisposeActiveToken(const void * promise);

        guiv2::Gui& Gui;
        lfs_t lfs{};
        lfs_config* disk;
        DeviceUsb& usb;
    private:
        std::vector<const void*> token_promises_;
    };
}

#endif //POCKETTRUSTEE_APP_PTT_HPP
