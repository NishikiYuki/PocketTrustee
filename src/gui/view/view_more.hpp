#ifndef POCKETTRUSTEE_VIEW_MORE_HPP
#define POCKETTRUSTEE_VIEW_MORE_HPP

#include "guiv2.hpp"
#include "gui_extension.hpp"
#include "app_ptt.hpp"

namespace guiv2::View::More {
    using namespace guiv2::extension;

    class MoreWindow final : public Window {
    public:
        static constexpr size_t kResWidth = 296;
        static constexpr size_t kResHeight = 128;
        static constexpr std::string_view kResPath = "/gui/more";

        static constexpr WindowRect kAreaHome = {{58, 98}, 32, 2};
        static constexpr WindowRect kAreaTransparent = {{128, 98}, 40, 2};
        static constexpr WindowRect kAreaManage = {{202, 98}, 47, 2};

        MoreWindow(Gui& gui, PocketTrustee::App& app);
        ~MoreWindow() override = default;

    protected:
        void OnCreate() override;
        void OnRender(Scheme& scheme) override;

        PocketTrustee::App& app_;
        MonoBitMap res_;

        ClickArea clickArea_Home;
        ClickArea clickArea_Transparent;
        ClickArea clickArea_Manage;

        pointer<Control> control_array_[3]{
            &clickArea_Home, &clickArea_Transparent, &clickArea_Manage
        };
    };
}

#endif //POCKETTRUSTEE_VIEW_MORE_HPP
