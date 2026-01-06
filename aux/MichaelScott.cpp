using Data = float;

struct Node;

struct Pointer {
    Node* node;
    unsigned int count;
};

struct Node {
    Data value;
    Pointer next;

    static Node Dummy() { return {{}, {nullptr, 0}}; }
};

struct Queue {
    Pointer Head;
    Pointer Tail;

    static Queue initialize()
    {
        Node* dummy = new Node(Node::Dummy());

        Pointer head{dummy, 0};
        Pointer tail{dummy, 0};
        return {head, tail};
    }

    void enqueue(Data value);
    bool dequeue(Data* pvalue);
};
