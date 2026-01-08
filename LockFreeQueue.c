#include "LockFreeQueue.h"
#include <stdatomic.h>

LockFreeQueue clfq_new(void)
{
    LockFreeQueue out;

    atomic_init(&out.front, 0);
    atomic_init(&out.back, 0);
    out.cached_front = 0;
    out.cached_back = 0;
    return out;
}

LockFreeQueueProducer clfq_producer(LockFreeQueue* clfq)
{
    return (LockFreeQueueProducer){.back = &clfq->back,
                                   .front = &clfq->front,
                                   .cached_front = &clfq->cached_front,
                                   .data = clfq->data};
}

static size_t distance(size_t front, size_t back)
{
    // scary underflow is actualy fine and expected behaviour
    // expects the queue size to be a power of 2
    return (back - front) & (CLF_QUEUE_SIZE - 1);
}
