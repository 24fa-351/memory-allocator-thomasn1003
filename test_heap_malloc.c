#include "heap_malloc.h"
#include <stdio.h>
#include <string.h>

void test_basic_allocations() {
    printf("Testing basic allocations...\n");
    void* a = heap_malloc(128);
    printf("Allocated 128 bytes at %p\n", a);
    void* b = heap_malloc(64);
    printf("Allocated 64 bytes at %p\n", b);
    void* c = heap_malloc(32);
    printf("Allocated 32 bytes at %p\n", c);

    heap_free(a);
    printf("Freed block at %p\n", a);

    void* d = heap_malloc(128);
    printf("Allocated 128 bytes at %p\n", d);

    heap_free(b);
    heap_free(c);
    heap_free(d);
}

int main() {
    if (heap_init(1024) != 0) {
        fprintf(stderr, "Heap initialization failed\n");
        return 1;
    }

    test_basic_allocations();

    heap_cleanup();
    return 0;
}
