#include <atomic>

// Michael-Scott "Lock-free" MPMC queue

// probably won't be using it in prod bc
// 1. linked list
// 2. not wait-free
// 3. allocates => actually not even lock-free

// might actually be lock-free with some kind of arena allocator

using Data = float;

struct Node;

struct Pointer {
    Node* node;
    unsigned int count;

    bool operator==(const Pointer& other) const
    {
        return this->node == other.node && this->count == other.count;
    }
};

// atomic as MS requires CAS instructions on it
struct AtomicPointer {
    static_assert(std::atomic<Pointer>::is_always_lock_free);

    std::atomic<Pointer> self;

    AtomicPointer(Node* p, unsigned int c) : self(Pointer{p, c}) {}

    void store(Pointer desired,
               std::memory_order order = std::memory_order_seq_cst) noexcept
    {
        this->self.store(desired, order);
    }

    Pointer load(
        std::memory_order order = std::memory_order_seq_cst) const noexcept
    {
        return this->self.load(order);
    }

    bool compare_exchange(
        Pointer& expected,
        Pointer desired,
        std::memory_order success = std::memory_order_seq_cst,
        std::memory_order failure = std::memory_order_seq_cst) noexcept
    {
        return self.compare_exchange_weak(expected, desired, success, failure);
    }
};

struct Node {
    Data value;
    AtomicPointer next;

    static Node Zeroed() { return {{}, AtomicPointer(nullptr, 0)}; }
};

struct Queue {
    AtomicPointer Head;
    AtomicPointer Tail;

    static Queue initialize()
    {
        Node* dummy = new Node(Node::Zeroed());

        return {{dummy, 0}, {dummy, 0}};
    }

    void enqueue(Data value)
    {
        Node* node = new Node(Node::Zeroed()); // E1, E3
        node->value = value;  // E2

        Pointer tail;
        Pointer next;

        while (1) {                         // E4
            tail = this->Tail.load();       // E5
            next = tail.node->next.load();  // E6

            // E7: is another Producer in the middle of a push?
            if (tail == this->Tail.load()) {
                // E8: is tail actually the tail ?
                if (next.node == nullptr) {
                    // E9: then we good: try to push using CAS
                    bool success = tail.node->next.compare_exchange(
                        next, {node, next.count + 1});
                    if (success)
                        break;  // E9-11
                } else {        // E12
                    // E13 tail is not the tail, maybe next is the actual tail
                    this->Tail.compare_exchange(tail,
                                                {next.node, tail.count + 1});
                }  // E14 endif
            }      // E15 endif
        }          // E16 endloop
        // E17 enqueue is done, update Tail
        this->Tail.compare_exchange(tail, {node, tail.count + 1});
    }

    bool dequeue(Data* pvalue);
};
