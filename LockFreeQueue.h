#pragma once

// Bounded SPSC Lock-Free Queue (Wait-Free even)
//
// Logic based on the WeakRB algorithm by Le et al. (2013),
// which formalizes and optimizes the original concurrent
// ring buffer principles established by Leslie Lamport (1983).
//
// meant for batch use (audio) rather than elementwise processing
//
// this is a Single Producer Single Consumer concurrent queue, so
// - Each LockFreeQueue supports exactly one producer thread and one consumer
// thread.
// - A LockFreeQueueProducer must be used by exactly one thread.
// - A LockFreeQueueConsumer must be used by exactly one thread.
// - Copying producer or consumer handles, or using them concurrently from
// multiple threads, results in undefined behavior.

#include <stdalign.h>
#include <stdbool.h>
#include <stdint.h>

// check that a size was i) supplied and ii) suitable
// also checks that atomic(u32) is always lock-free
// this allows us to use u32 everywhere
#include "LockFreeQueue_check.h"

// prevents false sharing on M-series CPU
// harmless padding on 64B systems
#define CACHE_LINE 128

// -------------------- Shared storage --------------------
typedef struct LockFreeQueue LockFreeQueue;
struct LockFreeQueue {
    // avoid false sharing
    alignas(CACHE_LINE) _Atomic(uint32_t) front;
    char pad1[CACHE_LINE - sizeof(_Atomic(uint32_t))];

    alignas(CACHE_LINE) _Atomic(uint32_t) back;
    char pad2[CACHE_LINE - sizeof(_Atomic(uint32_t))];

    alignas(CACHE_LINE) float data[CLF_QUEUE_SIZE];
};

// let the user choose the storage location
void clfq_new(LockFreeQueue* clfq);

// -------------------- Producer API --------------------
typedef struct LockFreeQueueProducer LockFreeQueueProducer;
struct LockFreeQueueProducer {
    _Atomic(uint32_t)* back;         // sole writer
    const _Atomic(uint32_t)* front;  // read only
    uint32_t cached_front;           // avoid pessimistic loads
    float* data;
};

LockFreeQueueProducer clfq_producer(LockFreeQueue* restrict clfq);

// pessimistic estimate using cached `front`. no atomic load at all
// there might be more available
uint32_t clfq_producer_size_lazy(
    const LockFreeQueueProducer* restrict producer);

// loads `front` with `acquire` ordering and updates `cached_front`
uint32_t clfq_producer_size_eager(LockFreeQueueProducer* restrict producer);

// push operations sometimes `acquire` load `front` when the pessimistic lazy
// size is not enough
// push operations always publish the new `back` with `release` ordering

// no partial transactions
bool clfq_push(LockFreeQueueProducer* restrict producer,
               const float* restrict elems,
               uint32_t n);
// push as many as possible, return samples written
// will only commit a multiple of frame_size as not to tear frames
// e.g. to preserve interleaved LR stereo frames, pass frame_size=2
uint32_t clfq_push_partial(LockFreeQueueProducer* restrict producer,
                           const float* restrict elems,
                           uint32_t n,
                           uint32_t frame_size);

// -------------------- Consumer API --------------------
// the API (and implementation) is pretty much symmetric, see Producer for info
typedef struct LockFreeQueueConsumer LockFreeQueueConsumer;
struct LockFreeQueueConsumer {
    _Atomic(uint32_t)* front;
    const _Atomic(uint32_t)* back;
    uint32_t cached_back;
    const float* data;
};

LockFreeQueueConsumer clfq_consumer(LockFreeQueue* restrict clfq);

uint32_t clfq_consumer_size_lazy(
    const LockFreeQueueConsumer* restrict consumer);
uint32_t clfq_consumer_size_eager(LockFreeQueueConsumer* restrict consumer);

bool clfq_pop(LockFreeQueueConsumer* restrict consumer,
              float* restrict elems,
              uint32_t n);
uint32_t clfq_pop_partial(LockFreeQueueConsumer* restrict consumer,
                          float* restrict elems,
                          uint32_t n,
                          uint32_t frame_size);

// peek returns a pointer to a contiguous slice of unread elements.
// The returned pointer remains valid until the next call that advances the
// consumer position (pop or skip). The producer will not overwrite peeked data
// until the consumer advances the front index.
//
// peek operations will always return a smaller size than consumer_size() as it
// has to guarantee a contiguous slice and so does not cross the buffer boundary

// Returns size of contiguous slice available using cached_back
// no load is performed
uint32_t clfq_consumer_peek_lazy(const LockFreeQueueConsumer* restrict consumer,
                                 const float** restrict ptr);

// Updates cached_back then returns contiguous slice size.
// `back` is loaded with `acquire` ordering
uint32_t clfq_consumer_peek_eager(LockFreeQueueConsumer* restrict consumer,
                                  const float** restrict ptr);

// unsafe, will cross the write end if not careful
// meant to be called after peek to consume data that we know is there
// the new read end is published with `release` ordering
void clfq_consumer_skip(LockFreeQueueConsumer* restrict consumer, uint32_t n);
