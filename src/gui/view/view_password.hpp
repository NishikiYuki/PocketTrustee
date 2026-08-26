#ifndef POCKETTRUSTEE_VIEW_PASSWORD_HPP
#define POCKETTRUSTEE_VIEW_PASSWORD_HPP

#include "guiv2.hpp"
#include "app_ptt.hpp"
#include "gui_extension.hpp"

namespace guiv2::View::Password {
    using namespace guiv2::extension;

    class PasswordWindow final : public Window {
    public:
        static constexpr size_t kMaxItemCount = 32;

        PasswordWindow(Gui& host, PocketTrustee::App& app);
        ~PasswordWindow() override = default;

    protected:
        void OnCreate() override;
        void OnEvent(const UiEvent& event) override { scroll_wn_.OnEvent(event); }
        void OnRender(Scheme& scheme) override { scroll_wn_.OnRender(scheme); }

        void ClickCallback(int n);

        PocketTrustee::App& app_;

        std::vector<std::string> items_;
        ScrollMenuWindow scroll_wn_;
    };
}


#endif //POCKETTRUSTEE_VIEW_PASSWORD_HPP
