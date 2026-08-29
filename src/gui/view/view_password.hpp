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

        void SyncWithDatabase();

        PocketTrustee::App& app_;

        std::vector<std::string> items_;
        ScrollMenuWindow scroll_wn_;
    };

    class PasswordGenWindow final : public Window {
    public:
        PasswordGenWindow(Gui& host, PocketTrustee::App& app);
        ~PasswordGenWindow() override;

        // Used to notify the parent to update its datebase;
        std::move_only_function<void()> OnClose;

    protected:
        void OnCreate() override;
        void OnRender(Scheme& scheme) override;

        void GeneratePassword();

        PocketTrustee::App& app_;
        std::string password_;
        bool contain_special_ = false;
        uint8_t password_length_ = 16;

        ClickArea password_area_;
        CheckBox special_checkbox_;
        NumberSelector length_selector_;
        Button button_back_;
        Button button_send_;
        Button button_save_;
        pointer<Control> control_array_[6]{
            &password_area_, &special_checkbox_, &length_selector_, &button_back_, &button_send_, &button_save_
        };
    };
}


#endif //POCKETTRUSTEE_VIEW_PASSWORD_HPP
