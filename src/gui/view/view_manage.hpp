#ifndef POCKETTRUSTEE_VIEW_MANAGE_HPP
#define POCKETTRUSTEE_VIEW_MANAGE_HPP

#include "guiv2.hpp"
#include "gui_extension.hpp"

namespace guiv2::View::Manage {
    using namespace guiv2::extension;

    class ManageWindow final : public InfoWindow {
    public:
        explicit ManageWindow(Gui& host);
        ~ManageWindow() override = default;
    };
}

#endif //POCKETTRUSTEE_VIEW_MANAGE_HPP
