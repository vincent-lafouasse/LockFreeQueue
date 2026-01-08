#pragma once

// SPSC Lock-Free Queue
//
// Logic based on the WeakRB algorithm by Le et al. (2013),
// which formalizes and optimizes the original concurrent
// ring buffer principles established by Leslie Lamport (1983).
//
// meant for batch use (audio) rather than elementwise processing

#include <stdalign.h>
#include <stdbool.h>
#include <stddef.h>

// prevents false sharing on M-series CPU
// harmless padding on 64B systems
#define CACHE_LINE 128
// 16KB
#define CLF_QUEUE_SIZE 4096

#define INVALID_QUEUE_SIZE_MSG                                      \
    "Concurrent lock free queue size must be a power of 2 in this " \
    "implementation"
_Static_assert(((CLF_QUEUE_SIZE & (CLF_QUEUE_SIZE - 1)) == 0),
               INVALID_QUEUE_SIZE_MSG);

// -------------------- Shared storage --------------------
typedef struct LockFreeQueue LockFreeQueue;
struct LockFreeQueue {
    // avoid false sharing
    alignas(CACHE_LINE) _Atomic(size_t) front;
    alignas(CACHE_LINE) _Atomic(size_t) back;
    alignas(CACHE_LINE) float data[CLF_QUEUE_SIZE];
};

LockFreeQueue clfq_new(void);

// -------------------- Producer API --------------------
typedef struct LockFreeQueueProducer LockFreeQueueProducer;
struct LockFreeQueueProducer {
    _Atomic(size_t)* back;         // sole writer
    const _Atomic(size_t)* front;  // read only
    size_t cached_front;           // avoid pessimistic loads
    float* data;
};

LockFreeQueueProducer clfq_producer(LockFreeQueue* clfq);

// pessimistic estimate using cached `front`
// there might be more available
size_t clfq_producer_size_lazy(const LockFreeQueueProducer* producer);

// loads `front` and updates `cached_front`
size_t clfq_producer_size_eager(LockFreeQueueProducer* producer);

// no partial transactions
bool clfq_push(LockFreeQueueProducer* restrict producer,
               const float* restrict elems,
               size_t n);
// push as many as possible, return samples written
size_t clfq_push_partial(LockFreeQueueProducer* restrict producer,
                         const float* restrict elems,
                         size_t n);

// -------------------- Consumer API --------------------
// the API (and implementation) is pretty much symmetric, see Producer for info
typedef struct LockFreeQueueConsumer LockFreeQueueConsumer;
struct LockFreeQueueConsumer {
    _Atomic(size_t)* front;
    const _Atomic(size_t)* back;
    size_t cached_back;
    const float* data;
};

LockFreeQueueConsumer clfq_consumer(LockFreeQueue* clfq);

size_t clfq_consumer_size_lazy(const LockFreeQueueConsumer* consumer);
size_t clfq_consumer_size_eager(LockFreeQueueConsumer* consumer);

bool clfq_pop(LockFreeQueueConsumer* consumer, float* restrict elems, size_t n);
size_t clfq_pop_partial(LockFreeQueueConsumer* restrict consumer,
                        float* restrict elems,
                        size_t n);

// peek operations will always return a smaller size than consumer_size() as it
// has to guarantee a contiguous slice

// Returns size of contiguous slice available using cached_back
size_t clfq_consumer_peek_lazy(const LockFreeQueueConsumer* consumer,
                               const float** ptr);

// Updates cached_back then returns contiguous slice size.
size_t clfq_consumer_peek_eager(LockFreeQueueConsumer* consumer,
                                const float** ptr);

// unsafe, will cross the write end if not careful
// meant to be called after peek
void clfq_consumer_skip(LockFreeQueueConsumer* consumer, size_t n);
