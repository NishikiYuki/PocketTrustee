#ifndef POCKETTRUSTEE_VIEW_STARTUP_HPP
#define POCKETTRUSTEE_VIEW_STARTUP_HPP

#include "guiv2.hpp"

namespace ViewStartup {
    using namespace guiv2;

    class StartupWindow final : public Window {
    public:
        explicit StartupWindow(Gui& host) : Window(host) {
            rect_.width = host.Width();
            rect_.height = host.Height();
            host.Invalidate(true);
        }
    protected:
        void OnRender(Scheme& scheme) override;
        void OnEvent(const UiEvent& event) override;
    };

    class InitializeDialog final : public Window {
    public:
        explicit InitializeDialog(Gui& host) : Window(host) {
            rect_.width = 128;
            rect_.height = 96;
            rect_.origin = Point2u::CenterOrientedOrigin(rect_.width, rect_.height,
                {host.Width() / 2, host.Height() / 2}
            );
        }
    protected:
        void OnRender(Scheme& scheme) override;
        void OnEvent(const UiEvent& event) override;
    };
}

#endif //POCKETTRUSTEE_VIEW_STARTUP_HPP
