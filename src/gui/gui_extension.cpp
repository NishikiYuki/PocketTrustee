#include "gui_extension.hpp"

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
    window.Text(
        Text, base_font::Ascii1608::Get(),
        Point2u::CenterOrientedOrigin(len, 16, Point2u::Center(Rect.origin, pos2))
    );
}

void OptionWindow::OnRender(Scheme& scheme) {
    scheme.Clear();
    Window::OnRender(scheme);
    scheme.Border();
    scheme.Line({0, 18}, {rect_.width - 1, 18}, 0);
    scheme.Text(Title, base_font::Ascii1608::Get(), {2, 1});
    scheme.Text(Text, base_font::Ascii1608::Get(), {2, 19});
}
