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

    Q[tail & (Size - 1)] = value;
    // no mod, the Lamport head and tail go off into infinity
    Tail.fetch_add(1, std::memory_order_release);
}

// assume single consumer
Data get()
{
    const size_t head = Head.load(std::memory_order_acquire);

    // spin while waiting for there to be anything in there ??
    while (head == Tail.load(std::memory_order_acquire)) {
    }

    const Data out = Q[head & (Size - 1)];
    // no mod, the Lamport head and tail go off into infinity
    Head.fetch_add(1, std::memory_order_release);
    return out;
}
}  // namespace Lamport
