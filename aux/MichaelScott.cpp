#include <atomic>

// probably won't be using it in prod bc
// 1. linked list
// 2. not wait-free
// 3. allocates => actually not even lock-free

using Data = float;

struct Node;

// atomic as MS requires CAS instructions on it
struct Pointer {
    struct Inner {
        Node* node;
        unsigned int count;
    };

    std::atomic<Inner> inner;
    static_assert(std::atomic<Inner>::is_always_lock_free);

    Pointer(Node* p, unsigned int c) : inner(Inner{p, c}) {}
};

struct Node {
    Data value;
    Pointer next;

    static Node Dummy() { return {{}, Pointer(nullptr, 0)}; }
};

struct Queue {
    Pointer Head;
    Pointer Tail;

    static Queue initialize()
    {
        Node* dummy = new Node(Node::Dummy());

        return {{dummy, 0}, {dummy, 0}};
    }

    void enqueue(Data value);
    bool dequeue(Data* pvalue);
};
