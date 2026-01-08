#pragma once

// SPSC Lock-Free Queue
//
// Logic based on the WeakRB algorithm by Le et al. (2013),
// which formalizes and optimizes the original concurrent
// ring buffer principles established by Leslie Lamport (1983).
//
// meant for batch use (audio) rather than elementwise processing

#include <stdatomic.h>
#include <stdbool.h>
#include <stddef.h>

// prevents false sharing on M-series CPU
// harmless padding on 64B systems
#define CACHE_LINE 128

#define CLF_QUEUE_SIZE 4096  // 16KB

#define INVALID_QUEUE_SIZE_MSG                                      \
    "Concurrent lock free queue size must be a power of 2 in this " \
    "implementation"
_Static_assert(((CLF_QUEUE_SIZE & (CLF_QUEUE_SIZE - 1)) == 0),
               INVALID_QUEUE_SIZE_MSG);

// An iteration on Le2013, itself an iteration from Lamport1983
typedef struct LockFreeQueue LockFreeQueue;
struct LockFreeQueue {
    // Consumer state, on a different cache line from Producer state
    // won't be invalidated by Producer thread
    _Alignas(CACHE_LINE) _Atomic size_t front;  // shared (S)

    // Producer state
    _Alignas(CACHE_LINE) _Atomic size_t back;

    _Alignas(CACHE_LINE) float data[CLF_QUEUE_SIZE];
};

LockFreeQueue clfq_new(void);

typedef struct LockFreeQueueProducer LockFreeQueueProducer;
struct LockFreeQueueProducer {
    _Atomic size_t* back;         // sole writer
    const _Atomic size_t* front;  // read only
    size_t cached_front;          // avoid pessimistic loads
    float* data;
};

LockFreeQueueProducer clfq_producer(LockFreeQueue* clfq);

// pessimistic estimate using cached `front`
// there might be more available
size_t clfq_producer_size_lazy(const LockFreeQueueProducer* producer);

// loads `front` and updates `cached_front`
size_t clfq_producer_size_eager(LockFreeQueueProducer* producer);

// no partial transactions
bool clfq_push(LockFreeQueueProducer* producer, const float* elems, size_t n);
// push as many as possible, return samples written
size_t clfq_push_partial(LockFreeQueueProducer* producer,
                         const float* elems,
                         size_t n);

typedef struct LockFreeQueueConsumer LockFreeQueueConsumer;
struct LockFreeQueueConsumer {
    _Atomic size_t* front;       // sole writer
    const _Atomic size_t* back;  // read only
    size_t cached_back;          // avoid pessimistic loads
    const float* data;
};

LockFreeQueueConsumer clfq_consumer(LockFreeQueue* clfq);

// no partial transactions
bool clfq_pop(LockFreeQueueConsumer* consumer, float* elems, size_t n);
// pop as many as possible, return samples read and consumed
size_t clfq_pop_partial(LockFreeQueueConsumer* consumer,
                        float* elems,
                        size_t n);
