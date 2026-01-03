#pragma once

#include <array>
#include <atomic>
#include <cassert>
#include <cstddef>
#include <stdexcept>

// spsc
template <class T, size_t N>
class LockFreeQueue {
   public:
    LockFreeQueue() : size_(0), read_pos_(0), write_pos_(0)
    {
        static_assert(N > 0 && (N & (N - 1)) == 0, "N must be a power of 2");
        static_assert(size_.is_always_lock_free,
                      "cannot guarantee size_t is always lock-free");  // C++17
        assert(size_.is_lock_free() &&
               "size_t is not lock-free on this platform");
    }

    // shared API
    auto size() const { return size_.load(); }

    // writer thread API
    auto push(const T& t)
    {
        if (size_.load() >= N) {
            throw std::overflow_error("queue is full");
        }
        buffer_[write_pos_] = t;
        write_pos_ =
            (write_pos_ + 1) & (N - 1);  // fast and correct is N is a pow2
        size_.fetch_add(1);
    }

    // reader API
    auto& front() const
    {
        auto s = size_.load();
        if (s == 0) {
            throw std::underflow_error("queue is empty");
        }
        return buffer_[read_pos_];
    }

    auto pop()
    {
        auto s = size_.load();
        if (s == 0) {
            throw std::underflow_error("queue is empty");
        }
        read_pos_ = (read_pos_ + 1) & (N - 1);
        size_.fetch_sub(1);
    }

   private:
    std::array<T, N> buffer_{};
    std::atomic<size_t> size_{};
    size_t read_pos_ = 0;   // not shared between threads
    size_t write_pos_ = 0;  // not shared between threads
};
