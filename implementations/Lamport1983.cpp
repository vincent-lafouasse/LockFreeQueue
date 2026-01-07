#include <array>
#include <atomic>
#include <cstddef>

// remarkably elegant and symmetric
// but the consumer thread probably shouldn't have access to he write end
// even if read-only => see Giacomoni2008

namespace Lamport {
using Data = float;

constexpr size_t Size = 1024;

static std::array<Data, Size> Q{};

// those should probably be on their own cache lines to avoid false sharing
static std::atomic<size_t> Head{0};
static std::atomic<size_t> Tail{0};

// assumes single producer
void put(Data value)
{
    // sole owner, relaxed is fine
    const size_t tail = Tail.load(std::memory_order_relaxed);

    // acquire because we want the latest Head value
    while (tail - Head.load(std::memory_order_acquire) == Size) {
        // spin while waiting for there to be space ??
    }

    Q[tail & (Size - 1)] = value;
    // no mod, the Lamport head and tail go off into infinity
    // `release` publishes the result to the consumer thread
    Tail.store(tail + 1, std::memory_order_release);
}

// assume single consumer
Data get()
{
    // sole owner, relaxed is fine
    const size_t head = Head.load(std::memory_order_relaxed);

    // acquire because we want the latest Tail value
    while (head == Tail.load(std::memory_order_acquire)) {
        // spin while waiting for there to be anything in there ??
    }

    const Data out = Q[head & (Size - 1)];
    // no mod, the Lamport head and tail go off into infinity
    // `release` publishes the result to the producer thread
    Head.store(head + 1, std::memory_order_release);
    return out;
}
}  // namespace Lamport
