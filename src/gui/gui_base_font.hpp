#ifndef POCKETTRUSTEE_GUI_BASE_FONT_HPP
#define POCKETTRUSTEE_GUI_BASE_FONT_HPP

#include "guiv2.hpp"

namespace guiv2::base_font {
    class Ascii1608 final : public IFont {
    public:
        ReadonlyMonoBitMap GetChar(char32_t ch) const override;

        static Ascii1608& Get();
    };
}

#endif //POCKETTRUSTEE_GUI_BASE_FONT_HPP
