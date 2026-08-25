#include "view_home.hpp"

using namespace guiv2::View::Home;

HomeWindow::HomeWindow(Gui& host, PocketTrustee::App& app) :
    Window(host, {Point2u{}, ResWidth, ResHeight}),
    app_(app),
    res_(ResWidth, ResHeight) {
    clickArea_Password.Rect = AreaPassword;
    clickArea_Token.Rect = AreaToken;
    clickArea_Card.Rect = AreaCard;
    clickArea_More.Rect = AreaMore;
    clickArea_Status.Rect = AreaStatus;
    SetControls(control_array_);
}

void HomeWindow::OnCreate() {
    PocketTrustee::LfsFileGuard file{&app_.lfs};
    if (file.open(ResPath.data(), LFS_O_RDONLY) != LFS_ERR_OK) {
        DEBUG_ERROR("failed to open /gui/home");
        return;
    }
    auto buffer = res_.getBuffer();
    lfs_file_read(file.lfs, &file.f, buffer.data(), buffer.size());
    host_.Invalidate(true);
}

void HomeWindow::OnRender(Scheme& scheme) {
    scheme.PasteFrom(res_, {});
    Window::OnRender(scheme);
}
