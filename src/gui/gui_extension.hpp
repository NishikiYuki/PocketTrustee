#ifndef POCKETTRUSTEE_GUI_EXTENSION_HPP
#define POCKETTRUSTEE_GUI_EXTENSION_HPP

#include <span>
#include <string>
#include "gui_base_font.hpp"
#include "guiv2.hpp"

namespace guiv2::extension {
    size_t CountGlyphs(std::string_view text);
    std::string TruncateText(std::string_view text, size_t max_glyphs);
    std::string MaskText(std::string_view text, size_t max_glyphs);

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

    class CheckBox final : public Control {
    public:
        CheckBox(const WindowRect& rect, std::string_view text, bool checked,
                 std::move_only_function<void(bool)> changed) :
            Rect(rect), Text(text), Checked(checked), OnChanged(std::move(changed)) {}

        CheckBox() = default;

        bool IsFocusable() const override { return true; }

        void OnRender(Scheme& window, Window& parent) override;
        bool OnEvent(const UiEvent& event, Window& parent) override;
        void OnFocusChanged(bool focused, Window& parent) override { focused_ = focused; }

        WindowRect Rect;
        std::string_view Text;
        bool Checked = false;
        std::move_only_function<void(bool)> OnChanged;

    private:
        bool focused_ = false;
    };

    class NumberSelector final : public Control {
    public:
        NumberSelector(const WindowRect& rect, std::string_view text, uint8_t value, uint8_t minimum,
                       uint8_t maximum, std::move_only_function<void(uint8_t)> changed) :
            Rect(rect), Text(text), Value(value), Minimum(minimum), Maximum(maximum), OnChanged(std::move(changed)) {}

        NumberSelector() = default;

        bool IsFocusable() const override { return true; }

        void OnRender(Scheme& window, Window& parent) override;
        bool OnEvent(const UiEvent& event, Window& parent) override;
        void OnFocusChanged(bool focused, Window& parent) override { focused_ = focused; }

        WindowRect Rect;
        std::string_view Text;
        uint8_t Value{};
        uint8_t Minimum{};
        uint8_t Maximum{};
        std::move_only_function<void(uint8_t)> OnChanged;

    private:
        bool focused_ = false;
        bool editing_ = false;
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

    class InfoWindow final : public Window {
    public:
        InfoWindow(Gui& host, const WindowRect& rect, std::string_view title, std::string_view text,
                   std::move_only_function<void()> callback) :
            Window(host, rect), Title(title), Text(text), Callback(std::move(callback)),
            button({}, {}, [this] { CallbackForward(); }) {
            constexpr size_t kButtonWidth = 20;
            constexpr size_t kButtonHeight = 12;
            const auto button_x = rect_.width > kButtonWidth ? (rect_.width - kButtonWidth) / 2 : 0;
            const auto button_y = rect_.height > 24 ? rect_.height - 24 : 0;
            button.Rect = {Point2u{button_x, button_y}, kButtonWidth, kButtonHeight};
            SetControls(control_array);
        }

        InfoWindow(Gui& host, const WindowRect& rect, std::string_view text,
                   std::move_only_function<void()> callback) :
            InfoWindow(host, rect, {}, text, std::move(callback)) {}

        std::string_view Title;
        std::string_view Text;
        std::string_view ButtonContent = "Ok";
        std::move_only_function<void()> Callback;

    protected:
        void OnRender(Scheme& scheme) override;

        Button button;
        Control* control_array[1]{&button};

        void CallbackForward() {
            if (Callback)
                Callback();
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

        // Intensionally exported Render and Event method
        void OnRender(Scheme& scheme) override; // NOLINTTHISLINE
        void OnEvent(const UiEvent& event) override; //NOLINTTHISLINE

    private:
        static constexpr size_t kNoSelection = 0xFFFFFFFF;
        static constexpr size_t kLineHeight = 16;
        static constexpr size_t kBorderWidth = 1;
        static constexpr size_t kScrollbarWidth = 3;
        static constexpr size_t kMinimumScrollbarThumbHeight = 8;
        static constexpr size_t kAsciiGlyphWidth = 8;

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
