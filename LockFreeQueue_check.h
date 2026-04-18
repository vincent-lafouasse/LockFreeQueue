#pragma once

#include "LockFreeQueue_conf.h"

#if !defined(CLF_QUEUE_SIZE)
#error "lockfreequeue_config.h must define LFQ_CAPACITY"
#endif

#define INVALID_QUEUE_SIZE_MSG                                      \
    "Concurrent lock free queue size must be a power of 2 in this " \
    "implementation"
_Static_assert(((CLF_QUEUE_SIZE & (CLF_QUEUE_SIZE - 1)) == 0),
               INVALID_QUEUE_SIZE_MSG);

#include <stdatomic.h>

// it's very important that the size type we use is _always_ lock-free
//
// (int is i32) and (ints are always lock-free) => i32 is always lock-free
// => u32 is always lock-free

_Static_assert(sizeof(int) == 4,
               "This codebase strictly requires 4-byte integers.");
_Static_assert(
    ATOMIC_INT_LOCK_FREE == 2,
    "32-bit operations are not natively lock-free on this platform.");

// now that we are assured that u32 are always lock-free
typedef uint32_t ClfqSizeType;
