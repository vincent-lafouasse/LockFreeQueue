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
