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

size_t clfq_producer_size_lazy(const LockFreeQueueProducer* producer)
{
    const size_t back =
        atomic_load_explicit(producer->back, memory_order_relaxed);
    return distance(back, producer->cached_front + CLF_QUEUE_SIZE);
}

size_t clfq_producer_size_eager(LockFreeQueueProducer* producer)
{
    producer->cached_front =
        atomic_load_explicit(producer->front, memory_order_acquire);
    return clfq_producer_size_lazy(producer);
}

bool clfq_push(LockFreeQueueProducer* producer,
               const float* restrict elems,
               size_t n)
{
    // check pessimistically, if it's fine do not reload `front`
    // we consider relaxed loading of private `back` free
    if (clfq_producer_size_lazy(producer) < n &&
        clfq_producer_size_eager(producer) < n) {
        return false;
    }

    // Producer is sole writer so there is no contention on `back`
    const size_t back =
        atomic_load_explicit(producer->back, memory_order_relaxed);

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

size_t clfq_consumer_size_lazy(const LockFreeQueueConsumer* consumer)
{
    const size_t front =
        atomic_load_explicit(consumer->front, memory_order_relaxed);

    return distance(front, consumer->cached_back);
}

size_t clfq_consumer_size_eager(LockFreeQueueConsumer* consumer)
{
    consumer->cached_back =
        atomic_load_explicit(consumer->back, memory_order_acquire);
    return clfq_consumer_size_lazy(consumer);
}

bool clfq_pop(LockFreeQueueConsumer* consumer, float* restrict elems, size_t n)
{
    if (clfq_consumer_size_lazy(consumer) < n &&
        clfq_consumer_size_eager(consumer) < n) {
        return false;
    }

    const size_t front =
        atomic_load_explicit(consumer->front, memory_order_relaxed);

    for (size_t i = 0; i < n; i++) {
        elems[i] = consumer->data[(front + i) & (CLF_QUEUE_SIZE - 1)];
    }

    atomic_store_explicit(consumer->front, front + n, memory_order_release);
    return true;
}
