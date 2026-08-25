#ifndef POCKETTRUSTEE_GUI_EXTENSION_HPP
#define POCKETTRUSTEE_GUI_EXTENSION_HPP

#include "guiv2.hpp"
#include "gui_base_font.hpp"

namespace guiv2::extension {
    class FontFamily {
    public:
        static FontFamily& LoadOrGetFontFamily();
    };

    class ClickArea : public Control {
    public:
        ClickArea(const WindowRect& rect, std::move_only_function<void()> click) :
            Rect(rect), OnClick(std::move(click)) {
        }

        ClickArea() = default;

        bool IsFocusable() const override { return true; }

        void OnRender(Scheme& window, Window& parent) override;
        bool OnEvent(const UiEvent& event, Window& parent) override;
        void OnFocusChanged(bool focused, Window& parent) override { focused_ = focused; }

        WindowRect Rect;
        std::move_only_function<void()> OnClick;
        uint8_t Border = 0;
        uint8_t BorderOnFocus = 1;

    protected:
        bool focused_ = false;
    };

    class Button final : public ClickArea {
    public:
        Button(const WindowRect& rect, std::string_view text, std::move_only_function<void()> click) :
            ClickArea(rect, std::move(click)), Text(text) {
        }

        Button() = default;

        void OnRender(Scheme& window, Window& parent) override;

        std::string_view Text;
    };

    class OptionWindow final : public Window {
    public:
        OptionWindow(Gui& host, const WindowRect& rect, std::string_view title, std::string_view text,
                     std::string_view option1, std::string_view option2, std::move_only_function<void (int)> callback) :
            Window(host, rect),
            Title(title),
            Text(text),
            Callback(std::move(callback)),
            btn1({}, option1, [this] { CallbackForward(1); }),
            btn2({}, option2, [this] { CallbackForward(2); }) {
            // calculate average padding
            size_t len = option1.size() + option2.size();
            len *= 8;
            len = (rect_.width - len) / 3;
            btn1.Rect = {
                Point2u{len, rect_.height - 24},
                option1.size() * 8 + 4,
                12
            };
            btn2.Rect = {
                Point2u{2 * len + btn1.Rect.width, rect_.height - 24},
                option2.size() * 8 + 4,
                12
            };
            SetControls(control_array);
        }

        std::string_view Title;
        std::string_view Text;
        std::move_only_function<void (int)> Callback;

    protected:
        void OnRender(Scheme& scheme) override;

        Button btn1, btn2;
        Control* control_array[2]{&btn1, &btn2};

        void CallbackForward(int n) {
            if (Callback) Callback(n);
            Close();
        }
    };
}

#endif //POCKETTRUSTEE_GUI_EXTENSION_HPP
