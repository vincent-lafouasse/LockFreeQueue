#include <array>
#include <atomic>
#include <cstddef>

namespace Lamport {
using Data = float;

constexpr size_t Size = 1024;  // number of elements

static std::array<Data, Size> Q{};

// those should probably be on their own cache lines to avoid false sharing
static std::atomic<size_t> Head{0};
static std::atomic<size_t> Tail{0};

// assumes single producer
void put(Data value)
{
    const size_t tail = Tail.load(std::memory_order_acquire);

    // spin while waiting for there to be space ??
    while (tail - Head.load(std::memory_order_acquire) == Size) {
    }

    Q[tail] = value;
    const size_t new_tail = (tail + 1) % Size;
    Tail.store(new_tail, std::memory_order_release);
}

// assume single consumer
Data get()
{
    const size_t head = Head.load(std::memory_order_acquire);

    // spin while waiting for there to be anything in there ??
    while (head == Tail.load(std::memory_order_acquire)) {
    }

    const Data out = Q[head];
    const size_t new_head = (head + 1) % Size;
    Head.store(new_head, std::memory_order_release);
    return out;
}
}  // namespace Lamport
