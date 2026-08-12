#ifndef POCKETTRUSTEE_SMALL_QUEUE_HPP
#define POCKETTRUSTEE_SMALL_QUEUE_HPP

#include <array>
#include <optional>

template <typename T, std::size_t N>
class SmallQueue {
public:
    static constexpr std::size_t capacity() noexcept {
        return N;
    }

    bool push(T value) {
        if (count_ == N) {
            return false;
        }

        std::size_t tail = (head_ + count_) % N;
        buf_[tail] = std::move(value);
        ++count_;

        return true;
    }

    std::optional<T> pop() {
        if (count_ == 0) {
            return std::nullopt;
        }

        T value = std::move(buf_[head_]);

        // 清空原槽位，避免 std::function 等对象继续持有资源
        buf_[head_] = T{};

        head_ = (head_ + 1) % N;
        --count_;

        return value;
    }

    T* front() {
        if (count_ == 0) {
            return nullptr;
        }
        return &buf_[head_];
    }

    const T* front() const {
        if (count_ == 0) {
            return nullptr;
        }
        return &buf_[head_];
    }

    bool empty() const noexcept {
        return count_ == 0;
    }

    bool full() const noexcept {
        return count_ == N;
    }

    std::size_t size() const noexcept {
        return count_;
    }

    void clear() {
        for (std::size_t i = 0; i < count_; ++i) {
            buf_[(head_ + i) % N] = T{};
        }

        head_ = 0;
        count_ = 0;
    }

private:
    std::array<T, N> buf_{};
    std::size_t head_ = 0;
    std::size_t count_ = 0;
};

#endif //POCKETTRUSTEE_SMALL_QUEUE_HPP
