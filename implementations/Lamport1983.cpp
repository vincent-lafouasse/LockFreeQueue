#include <array>
#include <atomic>
#include <cstddef>

// remarkably elegant and symmetric
// but the consumer thread probably shouldn't have access to he write end
// even if read-only => see Giacomoni2008
// the indices could be private instead of shared

// ## couple of changes from the pseudo-code
//
// - Lamport assumed complete sequential consistency and uses atomic registers
// (hence the shift based access instead of random access).
//
// - Sequential consistency is hard to expect from a weak memory order system so
// Head and Tail needed to be made atomic. I tried to match the semantics of the
// operations to C/C++ memory orders semantics
//
// - Lamport uses modulus, I fixed Size to a power of 2 to use bitwise AND instead.
// we gain a lot and lose nothing from having a power of 2 buffer size so i went for that
// (also makes it more usable in audio where buffer and fft sizes are almost always powers of 2)
//
// ## weird stuff that i let in:
//
// - the `skip` loop while empty/full
// this is an extremely busy wait that should probably be replaced by some kind
// of error value, e.g. `bool put(Data)` and `bool get(Data*)`
//
// - the Head and Tail indices never wrapped explicitely and going off into
// infinity this looks funny but the power of 2 Size should make Head and Tail
// overflow gracefully (since unlike signed overflow, unsigned overflow is
// perfectly standard and guaranteed)

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
