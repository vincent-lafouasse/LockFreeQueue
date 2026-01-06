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

    static Node Dummy() { return {{}, AtomicPointer(nullptr, 0)}; }
};

struct Queue {
    AtomicPointer Head;
    AtomicPointer Tail;

    static Queue initialize()
    {
        Node* dummy = new Node(Node::Dummy());

        return {{dummy, 0}, {dummy, 0}};
    }

    void enqueue(Data value)
    {
        Node node = Node::Dummy();  // E1, E3 (next is null)
        node.value = value;         // E2

        while (1) {  // E4
            Pointer tail = this->Head.load();
        }
    }

    bool dequeue(Data* pvalue);
};
