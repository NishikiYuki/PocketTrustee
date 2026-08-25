#ifndef POCKETTRUSTEE_VIEW_STARTUP_HPP
#define POCKETTRUSTEE_VIEW_STARTUP_HPP

#include "guiv2.hpp"

namespace guiv2::View::Startup {
    class StartupWindow final : public Window {
    public:
        explicit StartupWindow(Gui& host);
    protected:
        void OnCreate() override;
        void OnRender(Scheme& scheme) override;
        void OnEvent(const UiEvent& event) override;
    };
}

#endif //POCKETTRUSTEE_VIEW_STARTUP_HPP
