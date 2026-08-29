#include "view_password.hpp"
#include "view_home.hpp"
#include "app_password.hpp"
#include "usb_imp_hid.hpp"
#include <algorithm>

using namespace guiv2::View::Password;

PasswordWindow::PasswordWindow(Gui& host, PocketTrustee::App& app) :
    Window(host, WindowRect{{}, host.Width(), host.Height()}), app_(app),
    scroll_wn_(host, rect_, items_, [this](int n) { ClickCallback(n); }) {
    items_.reserve(8);
    items_.emplace_back("< Back");
    items_.emplace_back("+ New");
    scroll_wn_.SetItems(items_);
}

void PasswordWindow::OnCreate() {
    auto err = app_.PasswordVault.BuildIndex();
    auto index = app_.PasswordVault.GetEntityIndex();
    if (err != PocketTrustee::VaultDatabaseV1::Error::Ok) {
        goto fail;
    }
    for (auto& item : index) {
        std::string name;
        err = app_.PasswordVault.ReadName(item.entity_id, name);\
        if (err != PocketTrustee::VaultDatabaseV1::Error::Ok) {
            goto fail;
        }
        if (!name.empty()) {
            items_.push_back(name);
        }
    }
    return;
fail:
    Close();
    gui_.OpenWindowDeferred([gui = &gui_, app = &app_] {
        return std::make_unique<InfoWindow>(
            *gui, WindowRect{{84, 16}, 128, 96},
            "Database index build failed!",
            [&] {
                gui->OpenWindowDeferred([=] {
                    return std::make_unique<Home::HomeWindow>(*gui, *app);
                });
            });
    });
}

void PasswordWindow::ClickCallback(int n) {
    if (n == 0) {
        Close();
        gui_.OpenWindowDeferred(
            [host = &gui_, app = &app_] {
                return std::make_unique<Home::HomeWindow>(*host, *app);
            });
    }
    else if (n == 1) {
        gui_.OpenWindowDeferred(
            [host = &gui_, app = &app_] {
                return std::make_unique<PasswordGenWindow>(*host, *app);
            });
    }
}

PasswordGenWindow::PasswordGenWindow(Gui& host, PocketTrustee::App& app) :
    Window(host, {{63, 16}, 170, 96}), app_(app) {
    password_area_.Rect = {{1, 1}, 168, 34};
    password_area_.OnClick = [this] { GeneratePassword(); };

    special_checkbox_.Rect = {{8, 38}, 154, 16};
    special_checkbox_.Text = "Special chars";
    special_checkbox_.Checked = contain_special_;
    special_checkbox_.OnChanged = [this](bool checked) { contain_special_ = checked; };

    length_selector_.Rect = {{8, 54}, 154, 16};
    length_selector_.Text = "Length: ";
    length_selector_.Value = password_length_;
    length_selector_.Minimum = 6;
    length_selector_.Maximum = 32;
    length_selector_.OnChanged = [this](uint8_t value) { password_length_ = value; };

    button_back_.Rect = {{4, 76}, 42, 16};
    button_back_.Text = "Back";
    button_back_.OnClick = [this] { Close(); };

    button_send_.Rect = {{64, 76}, 42, 16};
    button_send_.Text = "Send";
    button_send_.OnClick = [this] { UsbHidDevice::GetInstance().TypeString(password_); };

    button_save_.Rect = {{124, 76}, 42, 16};
    button_save_.Text = "Save";
    button_save_.OnClick = [] {
    };

    SetControls(control_array_);
}

PasswordGenWindow::~PasswordGenWindow() {
    app_.PopUsbDevice();
}

void PasswordGenWindow::OnCreate() {
    app_.PopUsbDevice();
    app_.RegisterUsbDevice(&UsbHidDevice::GetInstance());
    GeneratePassword();
}

void PasswordGenWindow::OnRender(Scheme& scheme) {
    scheme.Clear();
    scheme.Border();

    constexpr size_t kGlyphWidth = 8;
    constexpr size_t kLineHeight = 16;
    constexpr size_t kMaxGlyphsPerLine = (168 - 4) / kGlyphWidth;
    for (size_t offset = 0, line = 0; offset < password_.size(); offset += kMaxGlyphsPerLine, ++line) {
        const auto line_length = std::min(kMaxGlyphsPerLine, password_.size() - offset);
        const auto x = password_area_.Rect.origin.x + (password_area_.Rect.width - line_length * kGlyphWidth) / 2;
        scheme.Text(std::string_view{password_}.substr(offset, line_length), base_font::Ascii1608::Get(),
                    {x, password_area_.Rect.origin.y + line * kLineHeight});
    }

    // Render controls last so the focus border remains intact around the password text.
    Window::OnRender(scheme);
}

void PasswordGenWindow::GeneratePassword() {
    const auto record = PasswordRecord::Generate({password_length_, contain_special_});
    password_ = record.Name;
    Invalidate();
}
