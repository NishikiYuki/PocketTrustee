#ifndef POCKETTRUSTEE_GUI_EXTENSION_HPP
#define POCKETTRUSTEE_GUI_EXTENSION_HPP

#include <span>
#include <string>
#include "gui_base_font.hpp"
#include "guiv2.hpp"

namespace guiv2::extension {
    class FontFamily {
    public:
        static FontFamily& LoadOrGetFontFamily();
    };

    class ClickArea : public Control {
    public:
        ClickArea(const WindowRect& rect, std::move_only_function<void()> click) :
            Rect(rect), OnClick(std::move(click)) {}

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
            ClickArea(rect, std::move(click)), Text(text) {}

        Button() = default;

        void OnRender(Scheme& window, Window& parent) override;

        std::string_view Text;
    };

    class OptionWindow final : public Window {
    public:
        OptionWindow(Gui& host, const WindowRect& rect, std::string_view title, std::string_view text,
                     std::string_view option1, std::string_view option2, std::move_only_function<void(int)> callback) :
            Window(host, rect), Title(title), Text(text), Callback(std::move(callback)),
            btn1({}, option1, [this] { CallbackForward(1); }), btn2({}, option2, [this] { CallbackForward(2); }) {
            // calculate average padding
            size_t len = option1.size() + option2.size();
            len *= 8;
            len = (rect_.width - len) / 3;
            btn1.Rect = {Point2u{len, rect_.height - 24}, option1.size() * 8 + 4, 12};
            btn2.Rect = {Point2u{2 * len + btn1.Rect.width, rect_.height - 24}, option2.size() * 8 + 4, 12};
            SetControls(control_array);
        }

        std::string_view Title;
        std::string_view Text;
        std::move_only_function<void(int)> Callback;

    protected:
        void OnRender(Scheme& scheme) override;

        Button btn1, btn2;
        Control* control_array[2]{&btn1, &btn2};

        void CallbackForward(int n) {
            if (Callback)
                Callback(n);
            Close();
        }
    };

    class ScrollMenuWindow final : public Window {
    public:
        ScrollMenuWindow(Gui& host, const WindowRect& rect, std::span<const std::string> items,
                         std::move_only_function<void(int)> callback) :
            Window(host, rect), Callback(std::move(callback)), items_(items) {}

        void SetItems(std::span<const std::string> items);
        std::span<const std::string> GetItems() const { return items_; }
        std::move_only_function<void(int)> Callback;

    protected:
        void OnRender(Scheme& scheme) override;
        void OnEvent(const UiEvent& event) override;

    private:
        static constexpr size_t kNoSelection = 0xFFFFFFFF;
        static constexpr size_t kLineHeight = 16;
        static constexpr size_t kBorderWidth = 1;
        static constexpr size_t kScrollbarWidth = 3;
        static constexpr size_t kMinimumScrollbarThumbHeight = 8;
        static constexpr size_t kAsciiGlyphWidth = 8;

        static size_t CountGlyphs(std::string_view text);
        static void RenderTextSingleLine(Scheme& scheme, std::string_view text, Point2u origin, size_t available_width);
        void MoveSelection(int direction);
        void EnsureSelectionVisible(size_t visible_item_count);
        void RenderScrollbar(Scheme& scheme, size_t visible_item_count);

        std::span<const std::string> items_;

        size_t selected_index_{items_.empty() ? kNoSelection : 0};
        size_t first_visible_index_{};
    };
} // namespace guiv2::extension

#endif // POCKETTRUSTEE_GUI_EXTENSION_HPP
