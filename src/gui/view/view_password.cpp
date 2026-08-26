#include "view_password.hpp"
#include "view_home.hpp"

using namespace guiv2::View::Password;

PasswordWindow::PasswordWindow(Gui& host, PocketTrustee::App& app) :
    Window(host, WindowRect{{}, host.Width(), host.Height()}), app_(app),
    scroll_wn_(host, rect_, items_, [this](int n) { ClickCallback(n); }) {
    items_.reserve(8);
}

void PasswordWindow::OnCreate() {
    items_.emplace_back("< Back");
    items_.emplace_back("+ New");
    scroll_wn_.SetItems(items_);
}

void PasswordWindow::ClickCallback(int n) {
    if (n == 0) {
        Close();
        gui_.OpenWindowDeferred(
            [host = &gui_, app = &app_] {
                return std::make_unique<Home::HomeWindow>(*host, *app);
            });
    }
}
