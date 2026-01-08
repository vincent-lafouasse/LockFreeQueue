#include "LockFreeQueue.h"

#include <stdatomic.h>

LockFreeQueue clfq_new(void)
{
    LockFreeQueue out;

    atomic_init(&out.front, 0);
    atomic_init(&out.back, 0);
    return out;
}

LockFreeQueueProducer clfq_producer(LockFreeQueue* clfq)
{
    return (LockFreeQueueProducer){.back = &clfq->back,
                                   .front = &clfq->front,
                                   .cached_front = 0,
                                   .data = clfq->data};
}

static size_t distance(size_t front, size_t back)
{
    // scary underflow is actualy fine and expected behaviour
    // expects the queue size to be a power of 2
    return (back - front) & (CLF_QUEUE_SIZE - 1);
}

bool clfq_push(LockFreeQueueProducer* producer, const float* elems, size_t n)
{
    // Producer is sole writer so there is no contention on `back`
    const size_t back =
        atomic_load_explicit(producer->back, memory_order_relaxed);

    // if the cached front already allows enough space for the transaction, do
    // not pessimistically load the latest front as the available size would
    // only get bigger with a fresher `front`
    if (distance(back, producer->cached_front + CLF_QUEUE_SIZE) < n) {
        // else we need to reload `front` and do this calculation again
        producer->cached_front =
            atomic_load_explicit(producer->front, memory_order_acquire);
        if (distance(back, producer->cached_front + CLF_QUEUE_SIZE) < n) {
            // 100% can't fit the whole buffer passed
            // TODO: partial version
            return false;
        }
    }

    for (size_t i = 0; i < n; ++i) {
        producer->data[(back + i) & (CLF_QUEUE_SIZE - 1)] = elems[i];
    }

    // publish/commit
    atomic_store_explicit(producer->back, back + n, memory_order_release);
    return true;
}

LockFreeQueueConsumer clfq_consumer(LockFreeQueue* clfq)
{
    return (LockFreeQueueConsumer){
        .front = &clfq->front,
        .back = &clfq->back,
        .cached_back = 0,
        .data = clfq->data,
    };
}

bool clfq_pop(LockFreeQueueConsumer* consumer, float* elems, size_t n)
{
    const size_t front =
        atomic_load_explicit(consumer->front, memory_order_relaxed);

    if (distance(front, consumer->cached_back) < n) {
        consumer->cached_back =
            atomic_load_explicit(consumer->back, memory_order_acquire);
        if (distance(front, consumer->cached_back) < n) {
            return false;
        }
    }

    for (size_t i = 0; i < n; i++) {
        elems[i] = consumer->data[(front + i) & (CLF_QUEUE_SIZE - 1)];
    }

    atomic_store_explicit(consumer->front, front + n, memory_order_release);
    return true;
}
