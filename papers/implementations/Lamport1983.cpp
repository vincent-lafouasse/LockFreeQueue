#include <array>
#include <atomic>
#include <cstddef>

// remarkably elegant and symmetric
// but the consumer thread probably shouldn't have access to he write end
// even if read-only => see Giacomoni2008
// the indices could be private instead of shared

// ## couple of changes from the pseudo-code
//
// - Lamport assumed complete sequential consistency, uses atomic registers and
// directly assigns them without mention memory fences (everything is
// implicitely seq_cst) so Head and Tail were made atomic<size_t>
//
// Sequential consistency is hard to expect from a weak memory order system so I
// tried to match the semantics of the operations to C/C++ memory orders
// semantics. But maybe i should have put `memory_seq_cst` for everybody to stay
// in the spirit of a Lamport queue since using relaxed and acquire/release is
// actually an optimization
//
// - Lamport uses modulus, I fixed Size to a power of 2 to use bitwise AND
// instead. we gain a lot and lose nothing from having a power of 2 buffer size
// so i went for that (also makes it more usable in audio where buffer and fft
// sizes are almost always powers of 2)
//
// ## weird stuff that i let in:
//
// - the `skip` loop while empty/full
//
// this is an extremely busy wait that should probably be replaced by some kind
// of error value, e.g. `bool put(Data)` and `bool get(Data*)`
//
// i guess this is fine for HFT where you need to snag a trade as soon as
// it's available but that's neither my interest nor my usecase
//
// - the Head and Tail indices never wrapped explicitely and going off into
// infinity
//
// this looks funny but the power of 2 Size should make Head and Tail
// overflow gracefully (since unlike signed overflow, unsigned overflow is
// perfectly standard and guaranteed)
//
// also we will probably never overflow the underlying u64, let's be honest

namespace Lamport {
using Data = float;

constexpr size_t Size = 1024;

static std::array<Data, Size> Q{};

// aligned as for each to be on its own cache line
// avoid false sharing
alignas(128) static std::atomic<size_t> Head{0};
alignas(128) static std::atomic<size_t> Tail{0};

// assumes single producer
void put(Data value)
{
    // sole owner, no risk of missing a write to Tail
    // `relaxed` is perfect
    const size_t tail = Tail.load(std::memory_order_relaxed);

    // acquire because we want the latest Head value
    while (tail - Head.load(std::memory_order_acquire) == Size) {
        // spin while waiting for there to be space
        // could return bool instead for a non-spinning/non-blocking version
    }

    Q[tail & (Size - 1)] = value;
    // no mod, the Lamport head and tail go off into infinity
    // `release` publishes the result to the consumer thread
    Tail.store(tail + 1, std::memory_order_release);
}

// assume single consumer
// every comment in `put` is applicable here, it's completely symmetric
Data get()
{
    const size_t head = Head.load(std::memory_order_relaxed);

    while (head == Tail.load(std::memory_order_acquire)) {
    }

    const Data out = Q[head & (Size - 1)];
    Head.store(head + 1, std::memory_order_release);
    return out;
}
}  // namespace Lamport
