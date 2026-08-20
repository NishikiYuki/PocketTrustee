#ifndef POCKETTRUSTEE_APP_PTT_HPP
#define POCKETTRUSTEE_APP_PTT_HPP

#include "app_ptt_utility.hpp"
#include "guiv2.hpp"

namespace PocketTrustee {
    struct IDevice {
        virtual ~IDevice() = default;
        virtual void Initialize() = 0;
        virtual std::string_view Name() const noexcept = 0;
    };

    struct AppComponents final {

    };

    class App final {
        App() = default;
    public:
        App(const App&) = delete;
        App& operator=(const App&) = delete;

        static App& GetApp();

        std::unique_ptr<guiv2::Gui> Gui;
        lfs_t lfs;
        lfs_config* disk;
    };
}

#endif //POCKETTRUSTEE_APP_PTT_HPP
