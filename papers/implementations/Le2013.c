#include <stdatomic.h>
#include <stdbool.h>

// a better Lamport queue

#define SIZE 1024
typedef float T;

// needs some alignment/padding to avoid false sharing
atomic_size_t front;  // write end
size_t pfront;        // thread private for the Producer

atomic_size_t back;  // read end
size_t cback;        // thread private for the Consumer

_Static_assert((SIZE & (SIZE - 1)) == 0, "SIZE must divide SIZE_MAX");

T data[SIZE];

void init(void)
{
    atomic_init(&front, 0);
    atomic_init(&back, 0);
}

bool push(const T* elems, size_t n)
{
    const size_t b = atomic_load_explicit(&back, memory_order_relaxed);

    // use cached front value
    if (pfront + SIZE - b < n) {
        pfront = atomic_load_explicit(&front, memory_order_acquire);
        if (pfront + SIZE - b < n) {
            return false;
        }
    }

    for (size_t i = 0; i < n; i++) {
        data[(b + i) & (SIZE - 1)] = elems[i];
    }

    atomic_store_explicit(&back, b + n, memory_order_release);
    return true;
}

bool pop(T* elems, size_t n)
{
    const size_t f = atomic_load_explicit(&front, memory_order_relaxed);

    if (cback - f < n) {
        cback = atomic_load_explicit(&back, memory_order_acquire);
        if (cback - f < n) {
            return false;
        }
    }

    for (size_t i = 0; i < n; i++) {
        elems[i] = data[(f + i) & (SIZE - 1)];
    }

    atomic_store_explicit(&front, f + n, memory_order_release);
    return true;
}
