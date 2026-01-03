#pragma once

#include <atomic>
#include <cassert>
#include <cstddef>
#include <memory>

// spsc
template <class T>
class LockFreeQueue {
   public:
    LockFreeQueue(size_t cap)
        : cap_(cap), mask_(cap - 1), size_(0), read_pos_(0), write_pos_(0)
    {
        static_assert(size_.is_always_lock_free,
                      "cannot guarantee size_t is always lock-free");  // C++17
        assert(size_.is_lock_free() &&
               "size_t is not lock-free on this platform");
        assert(cap > 0 && (cap & (cap - 1)) == 0 && "N must be a power of 2");
        buffer_ = std::make_unique<T[]>(cap);
    }

    // shared API
    auto size() const { return size_.load(); }

    // writer thread API
    bool push(const T& t)
    {
        if (size_.load() >= cap_) {
            return false;
        }
        buffer_[write_pos_] = t;
        write_pos_ = (write_pos_ + 1) & mask_;
        size_.fetch_add(1);
        return true;
    }

    // reader API
    T* front() const
    {
        auto s = size_.load();
        if (s == 0) {
            return nullptr;
        }
        return buffer_ + read_pos_;
    }

    bool pop()
    {
        auto s = size_.load();
        if (s == 0) {
            return false;
        }
        read_pos_ = (read_pos_ + 1) & mask_;
        size_.fetch_sub(1);
        return true;
    }

   private:
    const size_t cap_;
    const size_t mask_;
    std::unique_ptr<T[]> buffer_;

    std::atomic<size_t> size_{};
    size_t read_pos_ = 0;   // not shared between threads
    size_t write_pos_ = 0;  // not shared between threads
};
