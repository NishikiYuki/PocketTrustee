#ifndef POCKETTRUSTEE_APP_PTT_HPP
#define POCKETTRUSTEE_APP_PTT_HPP

#include "app_ptt_utility.hpp"
#include "guiv2.hpp"

namespace PocketTrustee {
    class App final {
    public:

    private:

    };

    struct AppComponents final {
        std::unique_ptr<guiv2::Gui> Gui;
    };

    class IDevice {
    public:
        virtual ~IDevice() = default;
        virtual void Initialize() = 0;
        virtual std::string_view Name() const noexcept = 0;
    };
}

#endif //POCKETTRUSTEE_APP_PTT_HPP
