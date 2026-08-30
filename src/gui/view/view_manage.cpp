#include "view_manage.hpp"

#include "app_shell.hpp"

using namespace guiv2::View::Manage;

ManageWindow::ManageWindow(Gui& host) :
    InfoWindow(host, WindowRect{{84, 16}, 128, 96}, "Manage", "On Manage Mode",
               [] { ShellSocket::GetInstance().Stop(); }) {
    ShellSocket::GetInstance().Start();
    ButtonContent = "Disconnect";
}
