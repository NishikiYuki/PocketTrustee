#ifndef POCKETTRUSTEE_VIEW_STARTUP_HPP
#define POCKETTRUSTEE_VIEW_STARTUP_HPP

#include "guiv2.hpp"

namespace ViewStartup {
    using namespace guiv2;

    class StartupWindow final : public Window {
    public:
        explicit StartupWindow(Gui& host) : Window(host, {{}, 296, 128}) {
            host.Invalidate(true);
        }
    protected:
        void OnRender(Scheme& scheme) override;
    };
}

#endif //POCKETTRUSTEE_VIEW_STARTUP_HPP
