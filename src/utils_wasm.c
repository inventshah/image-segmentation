#include "utils.h"

/**
 * This malloc implementation is a simple Linear Allocator. The address of
 * __heap_base represents the start of the heap. mem_pointer will move forward
 * on every allocation. Because allocation chunks are not tracked, free does
 * nothing. memory_reset should be used to deallocate everything.
 */

extern unsigned char __heap_base;
unsigned char *mem_pointer = &__heap_base;

EXPORT("malloc")
void *malloc(unsigned long n) {
    unsigned long remaining_size =
        __builtin_wasm_memory_size(0) * 64UL * 1024UL -
        (unsigned long)mem_pointer;
    if (remaining_size < n) {
        __builtin_wasm_memory_grow(0, (n - remaining_size) / 64UL / 1024UL + 1);
    }
    void *r = mem_pointer;
    mem_pointer += n;
    return r;
}

void free(void *p) { (void)p; }

EXPORT("memory_reset")
void memory_reset(void) { mem_pointer = &__heap_base; };

#include "api.h"