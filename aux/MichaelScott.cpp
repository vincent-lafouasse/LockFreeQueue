#include <atomic>

// probably won't be using it in prod bc
// 1. linked list
// 2. not wait-free
// 3. allocates => actually not even lock-free

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
        Node* node = new Node(Node::Zeroed());
        node->value = value;  // E2

        while (1) {                                 // E4
            Pointer tail = this->Tail.load();       // E5
            Pointer next = tail.node->next.load();  // E6

            // E7: is another Producer in the middle of a push?
            if (tail == this->Tail.load()) {
                // E8: is tail actually the tail ?
                if (next.node == nullptr) {
                    // E9: then we good: try to push using CAS
                    // if (this->Tail.compare_exchange())
                } else {
                }
            }
        }  // E16
    }

    bool dequeue(Data* pvalue);
};
