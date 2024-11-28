#ifndef HEAP_MALLOC_H
#define HEAP_MALLOC_H

#include <stddef.h>

int heap_init(size_t size);       // Initialize heap

void heap_cleanup();              // Cleanup heap memory

void* heap_malloc(size_t size);   // Allocate memory

void heap_free(void* ptr);        // Free allocated memory

void* heap_realloc(void* ptr, size_t size); // Reallocate memory

#endif
