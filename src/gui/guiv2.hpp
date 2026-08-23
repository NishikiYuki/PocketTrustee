#ifndef POCKETTRUSTEE_GUIV2_HPP
#define POCKETTRUSTEE_GUIV2_HPP

#include "common_log.h"
#include <cstdint>
#include <memory>
#include <span>
#include <string_view>
#include <vector>
#include <functional>
#include <small_queue.hpp>
#include "FreeRTOS.h"
#include "task.h"

namespace guiv2 {
    template <class T>
    using pointer = T*;

    struct Point2u {
        constexpr Point2u(size_t x, size_t y) : x(x), y(y) {
        }

        constexpr Point2u() : x(0), y(0) {
        }

        size_t x, y;

        Point2u operator+(const Point2u& other) const {
            return {x + other.x, y + other.y};
        }

        Point2u& operator+=(const Point2u& other) {
            x += other.x;
            y += other.y;
            return *this;
        }

        bool operator==(const Point2u& other) const {
            return x == other.x && y == other.y;
        }

        bool operator>(const Point2u& other) const {
            return x > other.x || y > other.y;
        }

        bool operator<(const Point2u& other) const {
            return x < other.x && y < other.y;
        }

        static constexpr Point2u Center(const Point2u& a, const Point2u& b) {
            return {(a.x + b.x) / 2, (a.y + b.y) / 2};
        }

        static constexpr Point2u CenterOrientedOrigin(size_t width, size_t height, const Point2u& center) {
            return {center.x - width / 2, center.y - height / 2};
        }
    };

    enum class EpdColor : uint8_t {
        kWhite = 1,
        kBlack = 0,
    };

    /**
     * @brief 只读BitMap
     */
    class ReadonlyMonoBitMap {
    public:
        constexpr ReadonlyMonoBitMap(uint16_t w, uint16_t h, const uint8_t* pBuffer) :
            width_(w), height_(h), pBuffer_(pBuffer), bufferSize_((width_ * height_ + 7) / 8) {
        }

        constexpr ReadonlyMonoBitMap(uint16_t w, uint16_t h, std::span<const uint8_t> pBuffer) :
            width_(w), height_(h), pBuffer_(pBuffer.data()), bufferSize_(pBuffer.size()) {
        }

        uint8_t operator[](Point2u pos) const;
        std::span<const uint8_t> getBuffer() const { return {pBuffer_, bufferSize_}; }
        constexpr size_t Width() const { return width_; }
        constexpr size_t Height() const { return height_; }

    protected:
        const size_t width_, height_;
        const uint8_t* pBuffer_;
        const size_t bufferSize_;
    };

    /**
     * @brief 为墨水屏提供支持
     *
     * @note TPixel = uint8_t 实际上一个字节存储8个像素
     * @note 如果EPD为垂直扫描,height % 8 == 0 的情况下,getBuffer可以直接被送往EPD,省去对齐过程
     */
    class MonoBitMap final : public ReadonlyMonoBitMap {
    public:
        MonoBitMap(uint16_t w, uint16_t h);
        MonoBitMap(uint16_t w, uint16_t h, uint8_t* pBuffer);
        ~MonoBitMap();

        MonoBitMap(const MonoBitMap&) = delete;
        MonoBitMap& operator=(const MonoBitMap&) = delete;

        MonoBitMap& Point(Point2u pos, uint8_t value);
        MonoBitMap& Clear();
        std::span<uint8_t> getBuffer() { return {pBuffer_, bufferSize_}; }

    private:
        const bool own_buffer_;
        uint8_t* pBuffer_;
    };

    /**
     * @brief 字体接口
     * @details 字体是通过index来索引FontChar对象的类
     */
    class IFont {
    public:
        virtual ~IFont() = default;
        /**
         * @brief 获取字符对应BitMap
         * @note 设计上来说支持Unicode,目前还只支持ASCII
         * @param ch Unicode 值
         * @return Readonly BitMap
         */
        virtual ReadonlyMonoBitMap GetChar(char32_t ch) const = 0;
    };

    /**
     * @brief 画板
     * @details 画板在BitMap的指定ROI区域内提供绘画方法
     *
     * @note Point方法是高频调用方法,动态多态影响性能,静态多态引入大量不必要的模板
     * @note 这里选择默认具有一个ROI
     */
    class Scheme {
    public:
        Scheme(MonoBitMap& map);
        Scheme(MonoBitMap& map, Point2u origin, size_t width, size_t height);

        Scheme(const Scheme&) = delete;
        Scheme& operator=(const Scheme&) = delete;

        Scheme& SetROI(Point2u origin, size_t width, size_t height);

        Scheme& Point(Point2u pos, uint8_t value) {
            map_.Point(pos + origin_, value);
            return *this;
        }

        uint8_t operator[](Point2u pos) const { return map_[pos + origin_]; }
        Scheme& Fill(Point2u pos1, Point2u pos2, uint8_t value);
        Scheme& Clear() { return Fill({}, {width_, height_}, 1); }
        Scheme& Line(Point2u pos1, Point2u pos2, uint8_t value);
        Scheme& Border(Point2u pos1, Point2u pos2, uint8_t value, uint8_t thickness = 1);
        Scheme& Border(uint8_t thickness = 1);
        Scheme& Invert(Point2u pos1, Point2u pos2);
        Scheme& PasteFrom(const Scheme& src, Point2u pos);
        Scheme& PasteFrom(const ReadonlyMonoBitMap& src, Point2u pos);
        Scheme& PasteTo(Scheme& dst, Point2u pos) const;
        Scheme& Text(std::string_view text, const IFont& font, Point2u pos);

        Point2u Origin() const { return origin_; }
        size_t Width() const { return width_; }
        size_t Height() const { return height_; }
        MonoBitMap& Map() { return map_; }

    protected:
        MonoBitMap& map_;
        Point2u origin_;
        size_t width_, height_;
    };

    class IDisplay {
    public:
        virtual ~IDisplay() = default;
        virtual size_t getWidth() const = 0;
        virtual size_t getHeight() const = 0;
        virtual void Update(const ReadonlyMonoBitMap& data, bool strong) = 0;
    };

    class UiEvent {
    public:
        enum BasicEvent {
            BasicUp = -1,
            BasicNull,
            BasicDown,
            BasicEnter
        };

        constexpr UiEvent(BasicEvent event = BasicNull) : bEvent(event) {
        }

        BasicEvent bEvent;
    };

    struct WindowRect {
        Point2u origin{};
        size_t width{};
        size_t height{};
    };

    class Gui;
    class Window;

    /**
     * @brief 控件基类
     */
    class Control {
    public:
        virtual ~Control() = default;
        virtual bool IsFocusable() const { return false; }

        /**
         * @brief 控件被要求进行渲染
         * @param window 父窗口画板
         * @param parent Parent
         */
        virtual void OnRender(Scheme& window, Window& parent) {
        }

        /**
         * @brief 当事件被送往控件时触发
         * @param event UiEvent
         * @param parent Parent
         * @return 如果控件响应Event,返回True
         */
        virtual bool OnEvent(const UiEvent& event, Window& parent) { return false; }

        /**
         * @brief 当窗体内焦点发生变更时触发
         * @param focused 自己是否被选中
         * @param parent 父窗口
         */
        virtual void OnFocusChanged(bool focused, Window& parent) {
        }
    };

    class Window {
    public:
        explicit Window(Gui& host, const WindowRect& rect) : host_(host), rect_(rect) {
        }

        explicit Window(Gui& host) : host_(host) {
        }

        virtual ~Window() = default;

        Window(const Window&) = delete;
        Window& operator=(const Window&) = delete;

        const WindowRect& Rect() const { return rect_; }

        Control* FocusedControl();
        const Control* FocusedControl() const;
        void SetControls(std::span<Control* const> controls);
        bool MoveControlFocus(int direction);
        bool FocusControl(Control& control);

        void Invalidate();

        void OpenWindow(std::unique_ptr<Window> window);
        void Close();

    protected:
        /**
         * @brief 当窗体被创建时触发
         */
        virtual void OnCreate() {
        }

        /**
         * @brief 要求窗体进行渲染
         * @param scheme ROI为窗体
         */
        virtual void OnRender(Scheme& scheme);

        /**
         * @brief 窗体处于焦点时,事件就会被发往窗体
         * @param event 事件
         */
        virtual void OnEvent(const UiEvent& event);

        /**
         * @brief 当窗体被关闭时触发
         */
        virtual void OnClose() {
        }

        friend class Gui;

        static constexpr size_t kNoControl = 0xFFFFFFFF;

        bool SetFocusedControl(size_t index);

        Gui& host_;
        WindowRect rect_;
        std::span<Control* const> controls_{};
        size_t focused_control_{kNoControl};
    };

    class Gui final {
    public:
        explicit Gui(IDisplay& display);
        ~Gui() = default;

        Gui(const Gui&) = delete;
        Gui& operator=(const Gui&) = delete;

        void OpenWindow(std::unique_ptr<Window> window);
        void Close(Window& window);

        void PostEvent(const UiEvent& event);

        /**
         * @brief Entrance of gui task
         */
        void Start(void*);

        void Invalidate(bool mark_as_strong = false);
        Window* FocusedWindow();
        const Window* FocusedWindow() const;

        size_t Width() const { return display_.getWidth(); }
        size_t Height() const { return display_.getHeight(); }

    private:
        void ApplyPendingOperations();
        void RenderIfNeeded();

        IDisplay& display_;
        MonoBitMap frameBuffer_;
        std::vector<std::unique_ptr<Window>> windows_;
        SmallQueue<std::move_only_function<void ()>, 8> pendingQueue_;
        UiEvent pendingEvent_;

        TaskHandle_t mainTaskHandle_{};

        bool dirty_{};
        bool mark_as_strong_{};
        volatile bool gui_busy_{};
    };
}

#endif //POCKETTRUSTEE_GUIV2_HPP
