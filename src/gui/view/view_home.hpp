#ifndef POCKETTRUSTEE_VIEW_HOME_HPP
#define POCKETTRUSTEE_VIEW_HOME_HPP

#include "guiv2.hpp"
#include "app_ptt.hpp"
#include "gui_extension.hpp"

namespace guiv2::View::Home {
    using namespace guiv2::extension;

    class HomeWindow final : public Window {
    public:
        static constexpr size_t ResWidth = 296;
        static constexpr size_t ResHeight = 128;
        static constexpr std::string_view ResPath = "/gui/home";
        static constexpr WindowRect AreaPassword = {{4, 111}, 68, 2};
        static constexpr WindowRect AreaToken = {{89, 111}, 44, 2};
        static constexpr WindowRect AreaCard = {{167, 111}, 37, 2};
        static constexpr WindowRect AreaMore = {{242, 111}, 35, 2};
        static constexpr WindowRect AreaStatus = {{224, 16}, 70, 2};

        explicit HomeWindow(Gui& host, PocketTrustee::App& app);
        ~HomeWindow() override = default;

    protected:
        void OnCreate() override;
        void OnRender(Scheme& scheme) override;

        PocketTrustee::App& app_;
        MonoBitMap res_;

        ClickArea clickArea_Password;
        ClickArea clickArea_Token;
        ClickArea clickArea_Card;
        ClickArea clickArea_More;
        ClickArea clickArea_Status;
        pointer<Control> control_array_[5]{
            &clickArea_Password, &clickArea_Token, &clickArea_Card, &clickArea_More, &clickArea_Status
        };
    };
}

#endif //POCKETTRUSTEE_VIEW_HOME_HPP
