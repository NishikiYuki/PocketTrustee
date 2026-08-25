#ifndef POCKETTRUSTEE_APP_PTT_HPP
#define POCKETTRUSTEE_APP_PTT_HPP

#include "app_ptt_utility.hpp"
#include "guiv2.hpp"
#include "driver_usb.hpp"

namespace PocketTrustee::Device {
    struct IRtcDevice {
        virtual ~IRtcDevice() = default;
        virtual void SetCount(uint64_t unix_time) = 0;
        virtual uint64_t GetCount() = 0;
    };

    struct ITrng {
        virtual ~ITrng() = default;
        virtual uint32_t Gen() = 0;
    };
}

namespace PocketTrustee {
    class App final {
        App(
            guiv2::Gui& gui,
            lfs_config* disk_,
            DeviceUsb& usb_,
            Device::IRtcDevice& rtc_,
            Device::ITrng& rng_
        );

    public:
        App(const App&) = delete;
        App& operator=(const App&) = delete;

        static App& InitOrGetApp();

        // To better manager the sleep behaviour, tasks now can take or dispose active token
        // Once no task have token, system is allowed to enter sleep mode
        void TakeActiveToken(const void * promise);
        void DisposeActiveToken(const void * promise);

        guiv2::Gui& Gui;
        lfs_t Lfs{};
        lfs_config* Disk;
        DeviceUsb& Usb;

        Device::IRtcDevice& Rtc;
        Device::ITrng& Rng;
    private:
        std::vector<const void*> token_promises_;
    };
}

#endif //POCKETTRUSTEE_APP_PTT_HPP
