#include "guiv2.hpp"
#include "FreeRTOS.h"
#include "task.h"

using namespace guiv2;

namespace {
    template <typename T>
    T clamp(T x, T min, T max) {
        if (x < min)
            return min;
        if (x > max)
            return max;
        return x;
    }

    /**
     * @brief 解码一个 UTF-8 字符,i 前进到下一个字符
     * @note 非法或截断的字节按原字节(0x00~0xFF)返回,保证 i 始终推进
     */
    char32_t DecodeUTF8(std::string_view s, size_t& i) {
        const auto b = static_cast<uint8_t>(s[i]);
        size_t len;
        char32_t cp;
        if (b < 0x80) {
            i++;
            return b;
        }
        if ((b & 0xE0) == 0xC0) {
            len = 2;
            cp = b & 0x1F;
        }
        else if ((b & 0xF0) == 0xE0) {
            len = 3;
            cp = b & 0x0F;
        }
        else if ((b & 0xF8) == 0xF0) {
            len = 4;
            cp = b & 0x07;
        }
        else {
            i++;
            return b;
        }
        if (i + len > s.size()) {
            i++;
            return b;
        }
        for (size_t k = 1; k < len; k++) {
            const auto c = static_cast<uint8_t>(s[i + k]);
            if ((c & 0xC0) != 0x80) {
                i++;
                return b;
            }
            cp = (cp << 6) | (c & 0x3F);
        }
        i += len;
        return cp;
    }
} // namespace

uint8_t ReadonlyMonoBitMap::operator[](Point2u pos) const {
    size_t bit = pos.x * height_ + pos.y;
    size_t idx = bit / 8;
    return !!(pBuffer_[idx] & (static_cast<uint8_t>(0x80) >> (bit % 8)));
}

MonoBitMap::MonoBitMap(uint16_t w, uint16_t h) : ReadonlyMonoBitMap(w, h, nullptr), own_buffer_(true) {
    pBuffer_ = new uint8_t[bufferSize_];
    ReadonlyMonoBitMap::pBuffer_ = pBuffer_;
    std::fill_n(pBuffer_, bufferSize_, 0xFF);
}

MonoBitMap::MonoBitMap(uint16_t w, uint16_t h, uint8_t* pBuffer) :
    ReadonlyMonoBitMap(w, h, pBuffer), own_buffer_(false), pBuffer_(pBuffer) {
    std::fill_n(pBuffer_, bufferSize_, 0xFF);
}

MonoBitMap::~MonoBitMap() {
    if (own_buffer_) {
        delete[] pBuffer_;
    }
}

MonoBitMap& MonoBitMap::Point(Point2u pos, uint8_t value) {
    size_t bit = pos.x * height_ + pos.y;
    size_t idx = bit / 8;
    auto mask = static_cast<uint8_t>(0x80u >> (bit % 8));
    if (value)
        pBuffer_[idx] |= mask;
    else
        pBuffer_[idx] &= ~mask;
    return *this;
}

MonoBitMap& MonoBitMap::Clear() {
    std::fill_n(pBuffer_, bufferSize_, 0xFF);
    return *this;
}

Scheme::Scheme(MonoBitMap& map) : map_(map), origin_({}), width_(map.Width()), height_(map.Height()) {}

Scheme::Scheme(MonoBitMap& map, Point2u origin, size_t width, size_t height) :
    map_(map), origin_(origin), width_(width), height_(height) {
    SetROI(origin, width, height);
}

Scheme& Scheme::SetROI(Point2u origin, size_t width, size_t height) {
    origin_.x = clamp<size_t>(origin.x, 0, map_.Width());
    origin_.y = clamp<size_t>(origin.y, 0, map_.Height());
    width_ = clamp<size_t>(width, 0, map_.Width() - origin_.x);
    height_ = clamp<size_t>(height, 0, map_.Height() - origin_.y);
    return *this;
}

Scheme& Scheme::Fill(Point2u pos1, Point2u pos2, uint8_t value) {
    for (size_t x = pos1.x; x < pos2.x && x < width_; x++) {
        for (size_t y = pos1.y; y < pos2.y && y < height_; y++) {
            Point({x, y}, value);
        }
    }
    return *this;
}

Scheme& Scheme::Line(Point2u pos1, Point2u pos2, uint8_t value) {
    int x0 = static_cast<int>(pos1.x), y0 = static_cast<int>(pos1.y);
    const int x1 = static_cast<int>(pos2.x), y1 = static_cast<int>(pos2.y);
    const int dx = abs(x1 - x0);
    const int dy = abs(y1 - y0);
    const int sx = (x0 < x1) ? 1 : -1;
    const int sy = (y0 < y1) ? 1 : -1;
    int err = dx - dy;
    for (;;) {
        if (x0 >= 0 && y0 >= 0 && static_cast<size_t>(x0) < width_ && static_cast<size_t>(y0) < height_)
            Point({static_cast<size_t>(x0), static_cast<size_t>(y0)}, value);
        if (x0 == x1 && y0 == y1)
            break;
        const int e2 = 2 * err;
        if (e2 > -dy) {
            err -= dy;
            x0 += sx;
        }
        if (e2 < dx) {
            err += dx;
            y0 += sy;
        }
    }
    return *this;
}

Scheme& Scheme::Border(Point2u pos1, Point2u pos2, uint8_t value, uint8_t thickness) {
    if (pos2.y < thickness || pos2.x < thickness) {
        return *this;
    }
    Fill(pos1, {pos2.x, pos1.y + thickness}, value);
    Fill({pos1.x, pos2.y - thickness}, pos2, value);
    Fill(pos1, {pos1.x + thickness, pos2.y}, value);
    Fill({pos2.x - thickness, pos1.y}, pos2, value);
    return *this;
}

Scheme& Scheme::Border(uint8_t thickness) {
    return Border({0, 0}, {width_, height_}, static_cast<uint8_t>(EpdColor::kBlack), thickness);
}

Scheme& Scheme::Invert(Point2u pos1, Point2u pos2) {
    for (size_t y = pos1.y; y < pos2.y && y < height_; y++) {
        for (size_t x = pos1.x; x < pos2.x && x < width_; x++) {
            Point({x, y}, static_cast<uint8_t>((*this)[{x, y}] ^ 1u));
        }
    }
    return *this;
}

Scheme& Scheme::PasteFrom(const Scheme& src, Point2u pos) {
    if (pos.x >= width_ || pos.y >= height_)
        return *this;
    const auto xCount = clamp<size_t>(src.width_, 0, width_ - pos.x);
    const auto yCount = clamp<size_t>(src.height_, 0, height_ - pos.y);
    if (xCount == 0 || yCount == 0)
        return *this;

    // 同一张 BitMap 上 ROI 重叠时,拷贝顺序决定正确性(等效 memmove):
    // 目标行在源行之下 → 自下而上;在源行之上 → 自上而下
    // 行完全重合时还需处理行内 x 方向的重叠
    const bool sameMap = (&src.map_ == &map_);
    const bool destBelow = sameMap && (origin_.y + pos.y > src.origin_.y);
    const bool destRight = sameMap && (origin_.y + pos.y == src.origin_.y) && (origin_.x + pos.x > src.origin_.x);

    auto copyRow = [&](size_t y, bool reversed) {
        if (reversed)
            for (size_t i = xCount; i > 0; i--)
                Point({pos.x + i - 1, pos.y + y}, src[{i - 1, y}]);
        else
            for (size_t x = 0; x < xCount; x++)
                Point({pos.x + x, pos.y + y}, src[{x, y}]);
    };

    if (destBelow) {
        for (size_t i = yCount; i > 0; i--)
            copyRow(i - 1, destRight);
    }
    else {
        for (size_t y = 0; y < yCount; y++)
            copyRow(y, destRight);
    }
    return *this;
}

Scheme& Scheme::PasteFrom(const ReadonlyMonoBitMap& src, Point2u pos) {
    if (pos.x >= width_ || pos.y >= height_)
        return *this;
    const auto xCount = clamp<size_t>(src.Width(), 0, width_ - pos.x);
    const auto yCount = clamp<size_t>(src.Height(), 0, height_ - pos.y);
    if (xCount == 0 || yCount == 0)
        return *this;

    for (size_t y = 0; y < yCount; y++) {
        for (size_t x = 0; x < xCount; x++) {
            Point({pos.x + x, pos.y + y}, src[{x, y}]);
        }
    }
    return *this;
}

Scheme& Scheme::PasteTo(Scheme& dst, Point2u pos) const {
    dst.PasteFrom(*this, pos);
    return dst;
}

Scheme& Scheme::Text(std::string_view text, const IFont& font, Point2u pos) {
    if (width_ == 0 || height_ == 0)
        return *this;
    if (pos.x >= width_ || pos.y >= height_)
        return *this;

    const size_t lineStartX = pos.x;
    for (size_t i = 0; i < text.size();) {
        const auto glyph = font.GetChar(DecodeUTF8(text, i));
        const size_t gWidth = glyph.Width();
        if (gWidth == 0)
            continue; // 字形缺失,不绘制也不推进

        // 当前字形放不下时换到下一行。字形本身比Scheme更宽时只能裁剪。
        if (pos.x >= width_ || (gWidth <= width_ && gWidth > width_ - pos.x)) {
            pos.x = lineStartX;
            pos.y += glyph.Height();
        }
        if (pos.y >= height_)
            break;

        const auto xEnd = clamp<size_t>(gWidth, 0, width_ - pos.x);
        const auto yEnd = clamp<size_t>(glyph.Height(), 0, height_ - pos.y);
        for (size_t y = 0; y < yEnd; y++) {
            for (size_t x = 0; x < xEnd; x++) {
                Point({pos.x + x, pos.y + y}, glyph[{x, y}]);
            }
        }
        pos.x += gWidth;
    }
    return *this;
}

Control* Window::FocusedControl() {
    if (focused_control_ >= controls_.size())
        return nullptr;
    return controls_[focused_control_];
}

const Control* Window::FocusedControl() const {
    if (focused_control_ >= controls_.size())
        return nullptr;
    return controls_[focused_control_];
}

void Window::SetControls(std::span<Control* const> controls) {
    if (auto* old = FocusedControl())
        old->OnFocusChanged(false, *this);

    controls_ = controls;
    focused_control_ = kNoControl;

    // 找到第一个Focusable控件
    for (size_t index = 0; index < controls_.size(); ++index) {
        if (controls_[index] != nullptr && controls_[index]->IsFocusable()) {
            focused_control_ = index;
            break;
        }
    }

    if (auto* current = FocusedControl(); current != nullptr)
        current->OnFocusChanged(true, *this);
    Invalidate();
}

bool Window::SetFocusedControl(size_t index) {
    if (index >= controls_.size() || controls_[index] == nullptr || !controls_[index]->IsFocusable() ||
        index == focused_control_)
        return false;

    if (auto* old = FocusedControl(); old != nullptr)
        old->OnFocusChanged(false, *this);
    focused_control_ = index;
    if (auto* current = FocusedControl(); current != nullptr)
        current->OnFocusChanged(true, *this);
    Invalidate();
    return true;
}

bool Window::MoveControlFocus(int direction) {
    if (direction == 0 || controls_.empty())
        return false;

    const bool forward = direction > 0;
    size_t index = focused_control_;
    if (index >= controls_.size())
        index = forward ? controls_.size() - 1 : 0;

    for (size_t count = 0; count < controls_.size(); ++count) {
        index = forward ? (index + 1) % controls_.size() : (index + controls_.size() - 1) % controls_.size();
        if (controls_[index] != nullptr && controls_[index]->IsFocusable())
            return SetFocusedControl(index);
    }
    return false;
}

bool Window::FocusControl(Control& control) {
    for (size_t index = 0; index < controls_.size(); ++index) {
        if (controls_[index] == &control)
            return SetFocusedControl(index);
    }
    return false;
}

void Window::OnRender(Scheme& scheme) {
    for (auto* control : controls_) {
        if (control != nullptr)
            control->OnRender(scheme, *this);
    }
}

void Window::OnEvent(const UiEvent& event) {
    if (auto* control = FocusedControl(); control != nullptr && control->OnEvent(event, *this))
        return;

    if (event.bEvent == UiEvent::BasicUp) {
        MoveControlFocus(-1);
    }
    else if (event.bEvent == UiEvent::BasicDown) {
        MoveControlFocus(1);
    }
}

void Window::Invalidate() { host_.Invalidate(); }

void Window::Close() { host_.Close(*this); }

void Window::OpenWindow(std::unique_ptr<Window> window) { host_.OpenWindow(std::move(window)); }

Gui::Gui(IDisplay& display) : display_(display), frameBuffer_(display.getWidth(), display.getHeight()) {
    windows_.reserve(4);
    pendingEventQueue_ = xQueueCreateStatic(kPendingEventCapacity, sizeof(UiEvent), pendingEventStorage_.data(),
                                            &pendingEventControlBlock_);
    DEBUG_ASSERT(pendingEventQueue_ != nullptr);
    DEBUG_INFO("Gui constructed");
}

void Gui::OpenWindow(std::unique_ptr<Window> window) {
    if (!window)
        return;
    pendingQueue_.push([this, wn = std::move(window)] mutable {
        wn->OnCreate();
        windows_.emplace_back(std::move(wn));
        Invalidate();
    });
}

void Gui::Close(Window& window) {
    pendingQueue_.push([this, target = &window] {
        auto it =
            std::ranges::find_if(windows_, [target](const std::unique_ptr<Window>& wn) { return wn.get() == target; });
        if (it == windows_.end())
            return;
        auto wn = std::move(*it);
        windows_.erase(it);
        wn.reset();
        Invalidate();
    });
}

void Gui::PostEventFromISR(const UiEvent& event) {
    BaseType_t higherPriorityTaskWoken = pdFALSE;
    if (xQueueSendFromISR(pendingEventQueue_, &event, &higherPriorityTaskWoken) == pdPASS) {
        portYIELD_FROM_ISR(higherPriorityTaskWoken);
    }
}

void Gui::Start(void*) {
    DEBUG_INFO("Gui start");
    UiEvent event{UiEvent::BasicNull};
    (void)xQueueSend(pendingEventQueue_, &event, 0);
    for (;;) {
        if (xQueueReceive(pendingEventQueue_, &event, portMAX_DELAY) != pdPASS)
            continue;
        if (auto focus = FocusedWindow()) {
            focus->OnEvent(event);
        }
        ApplyPendingOperations();
        RenderIfNeeded();
    }
}

void Gui::Invalidate(bool mark_as_strong) {
    if (mark_as_strong)
        mark_as_strong_ = true;
    dirty_ = true;
}

Window* Gui::FocusedWindow() { return windows_.empty() ? nullptr : windows_.back().get(); }

const Window* Gui::FocusedWindow() const { return windows_.empty() ? nullptr : windows_.back().get(); }

void Gui::ApplyPendingOperations() {
    while (!pendingQueue_.empty()) {
        auto pending = std::move(pendingQueue_.pop().value());
        pending();
    }
}

void Gui::RenderIfNeeded() {
    if (!dirty_)
        return;

    dirty_ = false;
    frameBuffer_.Clear();
    Scheme scheme{frameBuffer_};
    for (const auto& window : windows_) {
        const auto& [origin, width, height] = window->rect_;
        scheme.SetROI(origin, width, height);
        window->OnRender(scheme);
    }
    display_.Update(frameBuffer_, mark_as_strong_);
    mark_as_strong_ = false;
}
