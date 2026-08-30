#include "view_more.hpp"

#include "app_shell.hpp"
#include "view_home.hpp"

using namespace guiv2::View::More;

MoreWindow::MoreWindow(Gui& gui, PocketTrustee::App& app) :
    Window(gui, {{}, kResWidth, kResHeight}), app_(app), res_(kResWidth, kResHeight) {
    clickArea_Home.Rect = kAreaHome;
    clickArea_Home.OnClick = [this] {
        Close();
        gui_.OpenWindowDeferred([gui = &gui_, app = &app_] {
            return std::make_unique<Home::HomeWindow>(*gui, *app);
        });
    };
    clickArea_Transparent.Rect = kAreaTransparent;
    clickArea_Manage.Rect = kAreaManage;
    clickArea_Manage.OnClick = [this] {
        ShellSocket::GetInstance().Start();
        auto wn = std::make_unique<InfoWindow>(
            gui_, WindowRect{{84, 16}, 128, 96}, "Manage", "On Manage Mode",
            [] {
                ShellSocket::GetInstance().Stop();
            });
        wn->ButtonContent = "Disconnect";
        gui_.OpenWindow(std::move(wn));
    };
    SetControls(control_array_);
}

void MoreWindow::OnCreate() {
    PocketTrustee::LfsFileGuard file{&app_.Lfs};
    DEBUG_ASSERT(file.open(kResPath.data(), LFS_O_RDONLY) == LFS_ERR_OK);
    auto buffer = res_.getBuffer();
    lfs_file_read(file.lfs, &file.f, buffer.data(), buffer.size());
    gui_.Invalidate();
}

void MoreWindow::OnRender(Scheme& scheme) {
    scheme.PasteFrom(res_, {});
    Window::OnRender(scheme);
}
