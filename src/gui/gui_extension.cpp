#include "gui_extension.hpp"
#include <algorithm>

using namespace guiv2::extension;

void ClickArea::OnRender(Scheme& window, Window& parent) {
    auto pos2 = Rect.origin + Point2u{Rect.width, Rect.height};
    if (focused_ && BorderOnFocus) {
        window.Border(Rect.origin, pos2, 0, BorderOnFocus);
    }
    else if (Border) {
        window.Border(Rect.origin, pos2, 0, Border);
    }
}

bool ClickArea::OnEvent(const UiEvent& event, Window& parent) {
    if (event.bEvent == UiEvent::BasicEnter && OnClick) {
        OnClick();
        return true;
    }
    return false;
}

void Button::OnRender(Scheme& window, Window& parent) {
    ClickArea::OnRender(window, parent);
    auto pos2 = Rect.origin + Point2u{Rect.width, Rect.height};
    size_t len = Text.size() * 8;
    window.Text(Text, base_font::Ascii1608::Get(),
                Point2u::CenterOrientedOrigin(len, 16, Point2u::Center(Rect.origin, pos2)));
}

void CheckBox::OnRender(Scheme& window, Window& parent) {
    constexpr size_t kMarkSize = 8;
    constexpr size_t kTextGap = 4;
    const auto pos2 = Rect.origin + Point2u{Rect.width, Rect.height};
    const auto mark_origin = Rect.origin + Point2u{4, 4};
    const auto mark_end = mark_origin + Point2u{kMarkSize, kMarkSize};
    window.Border(mark_origin, mark_end, 0);
    if (Checked)
        window.Fill({mark_origin.x + 1, mark_origin.y + 1}, {mark_end.x - 1, mark_end.y - 1}, 0);
    window.Text(Text, base_font::Ascii1608::Get(), {mark_end.x + kTextGap, Rect.origin.y});
    if (focused_)
        window.Border(Rect.origin, pos2, 0, 1);
}

bool CheckBox::OnEvent(const UiEvent& event, Window& parent) {
    if (event.bEvent != UiEvent::BasicEnter)
        return false;
    Checked = !Checked;
    if (OnChanged)
        OnChanged(Checked);
    parent.Invalidate();
    return true;
}

void NumberSelector::OnRender(Scheme& window, Window& parent) {
    const auto pos2 = Rect.origin + Point2u{Rect.width, Rect.height};
    char value_text[4]{};
    size_t value_length = 1;
    if (Value >= 100) {
        value_text[0] = static_cast<char>('0' + Value / 100);
        value_text[1] = static_cast<char>('0' + (Value / 10) % 10);
        value_text[2] = static_cast<char>('0' + Value % 10);
        value_length = 3;
    }
    else if (Value >= 10) {
        value_text[0] = static_cast<char>('0' + Value / 10);
        value_text[1] = static_cast<char>('0' + Value % 10);
        value_length = 2;
    }
    else {
        value_text[0] = static_cast<char>('0' + Value);
    }

    window.Text(Text, base_font::Ascii1608::Get(), Rect.origin);
    window.Text(std::string_view{value_text, value_length}, base_font::Ascii1608::Get(),
                {Rect.origin.x + Text.size() * 8, Rect.origin.y});
    if (editing_)
        window.Invert(Rect.origin, pos2);
    if (focused_)
        window.Border(Rect.origin, pos2, 0, 1);
}

bool NumberSelector::OnEvent(const UiEvent& event, Window& parent) {
    if (event.bEvent == UiEvent::BasicEnter) {
        editing_ = !editing_;
        parent.Invalidate();
        return true;
    }
    if (!editing_ || (event.bEvent != UiEvent::BasicUp && event.bEvent != UiEvent::BasicDown))
        return false;

    const auto old_value = Value;
    if (event.bEvent == UiEvent::BasicUp && Value < Maximum)
        ++Value;
    else if (event.bEvent == UiEvent::BasicDown && Value > Minimum)
        --Value;
    if (Value != old_value && OnChanged)
        OnChanged(Value);
    parent.Invalidate();
    return true;
}

void OptionWindow::OnRender(Scheme& scheme) {
    scheme.Clear();
    Window::OnRender(scheme);
    scheme.Border();
    scheme.Line({0, 18}, {rect_.width - 1, 18}, 0);
    scheme.Text(Title, base_font::Ascii1608::Get(), {2, 1});
    scheme.Text(Text, base_font::Ascii1608::Get(), {2, 19});
}

void InfoWindow::OnRender(Scheme& scheme) {
    scheme.Clear();
    Window::OnRender(scheme);
    scheme.Border();
    scheme.Text(Text, base_font::Ascii1608::Get(), {2, 1});
}

namespace guiv2::extension {

size_t CountGlyphs(std::string_view text) {
    size_t count = 0;
    for (size_t index = 0; index < text.size();) {
        DecodeUTF8(text, index);
        count++;
    }
    return count;
}

std::string TruncateText(std::string_view text, size_t max_glyphs) {
    if (CountGlyphs(text) <= max_glyphs)
        return std::string{text};

    constexpr std::string_view ellipsis = "...";
    if (max_glyphs <= ellipsis.size())
        return std::string{ellipsis.substr(0, max_glyphs)};

    const auto content_glyphs = max_glyphs - ellipsis.size();
    size_t index = 0;
    size_t end = 0;
    for (size_t rendered = 0; rendered < content_glyphs && index < text.size(); ++rendered) {
        DecodeUTF8(text, index);
        end = index;
    }
    std::string result{text.substr(0, end)};
    result += ellipsis;
    return result;
}

std::string MaskText(std::string_view text, size_t max_glyphs) {
    const auto glyphs = CountGlyphs(text);
    if (glyphs == 0)
        return "(None)";
    return std::string(std::min(glyphs, max_glyphs), '*');
}

} // namespace guiv2::extension

void ScrollMenuWindow::RenderTextSingleLine(Scheme& scheme, std::string_view text, Point2u origin,
                                            size_t available_width) {
    const auto max_glyph_count = available_width / kAsciiGlyphWidth;
    if (max_glyph_count == 0)
        return;

    const auto glyph_count = guiv2::extension::CountGlyphs(text);
    const bool truncated = glyph_count > max_glyph_count;
    const auto ellipsis_count = truncated ? std::min<size_t>(3, max_glyph_count) : 0;
    const auto text_glyph_limit = max_glyph_count - ellipsis_count;
    auto& font = base_font::Ascii1608::Get();

    size_t x = origin.x;
    size_t rendered = 0;
    for (size_t index = 0; index < text.size() && rendered < text_glyph_limit; rendered++) {
        const auto codepoint = DecodeUTF8(text, index);
        scheme.PasteFrom(font.GetChar(codepoint), {x, origin.y});
        x += kAsciiGlyphWidth;
    }

    if (!truncated)
        return;

    for (size_t i = 0; i < ellipsis_count; i++) {
        scheme.PasteFrom(font.GetChar('.'), {x, origin.y});
        x += kAsciiGlyphWidth;
    }
}

void ScrollMenuWindow::OnRender(Scheme& scheme) {
    scheme.Clear();

    if (scheme.Width() <= 2 * kBorderWidth + kScrollbarWidth || scheme.Height() <= 2 * kBorderWidth) {
        return;
    }

    const auto content_width = scheme.Width() - 2 * kBorderWidth - kScrollbarWidth;
    const auto visible_item_count = (scheme.Height() - 2 * kBorderWidth) / kLineHeight;
    EnsureSelectionVisible(visible_item_count);

    for (size_t row = 0; row < visible_item_count; row++) {
        const auto item_index = first_visible_index_ + row;
        if (item_index >= items_.size())
            break;

        const auto origin = Point2u{kBorderWidth, kBorderWidth + row * kLineHeight};
        RenderTextSingleLine(scheme, items_[item_index], origin, content_width);
        if (item_index == selected_index_) {
            scheme.Invert(origin, {origin.x + content_width, origin.y + kLineHeight});
        }
    }

    RenderScrollbar(scheme, visible_item_count);
    scheme.Border(kBorderWidth);
}

void ScrollMenuWindow::SetItems(std::span<const std::string> items) {
    items_ = items;

    if (items_.empty()) {
        selected_index_ = kNoSelection;
    }
    else if (selected_index_ == kNoSelection || selected_index_ >= items_.size()) {
        selected_index_ = items_.size() - 1;
    }

    const auto visible_item_count =
        rect_.height > 2 * kBorderWidth ? (rect_.height - 2 * kBorderWidth) / kLineHeight : 0;
    EnsureSelectionVisible(visible_item_count);
    Invalidate();
}

void ScrollMenuWindow::OnEvent(const UiEvent& event) {
    if (items_.empty())
        return;

    if (event.bEvent == UiEvent::BasicUp) {
        MoveSelection(-1);
    }
    else if (event.bEvent == UiEvent::BasicDown) {
        MoveSelection(1);
    }
    else if (event.bEvent == UiEvent::BasicEnter && Callback) {
        Callback(static_cast<int>(selected_index_));
    }
}

void ScrollMenuWindow::MoveSelection(int direction) {
    if (items_.empty() || direction == 0)
        return;

    if (selected_index_ == kNoSelection) {
        selected_index_ = 0;
    }
    else if (direction < 0) {
        selected_index_ = selected_index_ == 0 ? items_.size() - 1 : selected_index_ - 1;
    }
    else {
        selected_index_ = selected_index_ + 1 == items_.size() ? 0 : selected_index_ + 1;
    }

    const auto visible_item_count =
        rect_.height > 2 * kBorderWidth ? (rect_.height - 2 * kBorderWidth) / kLineHeight : 0;
    EnsureSelectionVisible(visible_item_count);
    Invalidate();
}

void ScrollMenuWindow::EnsureSelectionVisible(size_t visible_item_count) {
    if (items_.empty() || selected_index_ == kNoSelection || visible_item_count == 0) {
        first_visible_index_ = 0;
        return;
    }

    if (selected_index_ < first_visible_index_) {
        first_visible_index_ = selected_index_;
    }
    else if (selected_index_ >= first_visible_index_ + visible_item_count) {
        first_visible_index_ = selected_index_ - visible_item_count + 1;
    }

    const auto max_first_visible = items_.size() > visible_item_count ? items_.size() - visible_item_count : 0;
    if (first_visible_index_ > max_first_visible)
        first_visible_index_ = max_first_visible;
}

void ScrollMenuWindow::RenderScrollbar(Scheme& scheme, size_t visible_item_count) {
    const auto track_origin = Point2u{
        scheme.Width() - kBorderWidth - kScrollbarWidth,
        kBorderWidth,
    };
    const auto track_height = scheme.Height() - 2 * kBorderWidth;
    const auto track_end = track_origin + Point2u{kScrollbarWidth, track_height};
    const auto inner_track_height = track_height > 2 ? track_height - 2 : 0;

    scheme.Border(track_origin, track_end, static_cast<uint8_t>(EpdColor::kBlack));

    if (inner_track_height == 0)
        return;

    size_t thumb_height = inner_track_height;
    size_t thumb_offset = 0;
    if (visible_item_count != 0 && items_.size() > visible_item_count) {
        thumb_height = inner_track_height * visible_item_count / items_.size();
        thumb_height = std::max(thumb_height, kMinimumScrollbarThumbHeight);
        thumb_height = std::min(thumb_height, inner_track_height);

        const auto scroll_range = items_.size() - visible_item_count;
        thumb_offset = (inner_track_height - thumb_height) * first_visible_index_ / scroll_range;
    }

    const auto thumb_origin = track_origin + Point2u{1, thumb_offset + 1};
    const auto thumb_end = thumb_origin + Point2u{kScrollbarWidth - 2, thumb_height};
    scheme.Fill(thumb_origin, thumb_end, static_cast<uint8_t>(EpdColor::kBlack));
}
