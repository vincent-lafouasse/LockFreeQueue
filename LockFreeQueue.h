#pragma once

#include <stdatomic.h>
#include <stddef.h>

// prevents false sharing on M-series CPU
// harmless padding on 64B systems
#define CACHE_LINE 128

#define CLF_QUEUE_SIZE 1024

#define INVALID_QUEUE_SIZE_MSG                                      \
    "Concurrent lock free queue size must be a power of 2 in this " \
    "implementation"
_Static_assert(((CLF_QUEUE_SIZE & (CLF_QUEUE_SIZE - 1)) == 0),
               INVALID_QUEUE_SIZE_MSG);

// An iteration on Le2013, itself an iteration from Lamport1983
typedef struct LockFreeQueue LockFreeQueue;
struct LockFreeQueue {
    // Consumer state, on a different cache line from Producer state
    _Alignas(CACHE_LINE) _Atomic size_t front;  // shared (S)
    // private to Consumer, won't be invalidated by Producer thread
    size_t cached_back;  // shared, not exclusive (E), didn't want to waste more
                         // memory

    // Producer state
    _Alignas(CACHE_LINE) _Atomic size_t back;
    size_t cached_front;

    _Alignas(CACHE_LINE) float data[CLF_QUEUE_SIZE];
};

typedef struct LockFreeQueueProducer LockFreeQueueProducer;
struct LockFreeQueueProducer {
    _Atomic size_t* back;         // sole writer
    const _Atomic size_t* front;  // read only
    size_t* cached_front;
    float* data;
};

LockFreeQueue clfq_new(void);

LockFreeQueueProducer clfq_producer(LockFreeQueue* clfq);
