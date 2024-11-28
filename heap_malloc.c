#include "heap_malloc.h"
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>

typedef struct Block {
    size_t size;            // Size of the block
    int is_free;            // Free status: 1 if free, 0 otherwise
    struct Block* next;     // Pointer to the next block
} Block;

static void* heap_start = NULL;   // Start of the managed heap
static Block* free_list = NULL;   // Free list head
static size_t heap_size = 0;      // Total size of the heap
static pthread_mutex_t heap_lock = PTHREAD_MUTEX_INITIALIZER;

// Align size to 8 bytes
static size_t align(size_t size) {
    return (size + 7) & ~7;
}

// Find a free block
static Block* find_free_block(size_t size) {
    Block* current = free_list;
    while (current) {
        if (current->is_free && current->size >= size) {
            return current;
        }
        current = current->next;
    }
    return NULL;
}

// Extend heap with a new block
static Block* extend_heap(size_t size) {
    void* block_start = sbrk(size + sizeof(Block));
    if (block_start == (void*)-1) {
        fprintf(stderr, "[ERROR] sbrk failed\n");
        return NULL;
    }

    Block* new_block = (Block*)block_start;
    new_block->size = size;
    new_block->is_free = 0;
    new_block->next = NULL;

    return new_block;
}

// Split block if possible
static void split_block(Block* block, size_t size) {
    if (block->size >= size + sizeof(Block) + 8) {
        Block* new_block = (Block*)((char*)block + sizeof(Block) + size);
        new_block->size = block->size - size - sizeof(Block);
        new_block->is_free = 1;
        new_block->next = block->next;

        block->size = size;
        block->next = new_block;
    }
}

int heap_init(size_t size) {
    pthread_mutex_lock(&heap_lock);

    heap_start = sbrk(size);
    if (heap_start == (void*)-1) {
        pthread_mutex_unlock(&heap_lock);
        return -1;
    }

    heap_size = size;
    free_list = (Block*)heap_start;
    free_list->size = size - sizeof(Block);
    free_list->is_free = 1;
    free_list->next = NULL;

    pthread_mutex_unlock(&heap_lock);
    return 0;
}

void heap_cleanup() {
    pthread_mutex_lock(&heap_lock);
    heap_start = NULL;
    free_list = NULL;
    heap_size = 0;
    pthread_mutex_unlock(&heap_lock);
}

void* heap_malloc(size_t size) {
    if (size == 0) {
        return NULL;
    }

    pthread_mutex_lock(&heap_lock);

    size = align(size);
    Block* block = find_free_block(size);
    if (!block) {
        block = extend_heap(size);
        if (!block) {
            pthread_mutex_unlock(&heap_lock);
            return NULL;
        }
    } else {
        block->is_free = 0;
        split_block(block, size);
    }

    pthread_mutex_unlock(&heap_lock);
    return (void*)((char*)block + sizeof(Block));
}

void heap_free(void* ptr) {
    if (!ptr) {
        return;
    }

    pthread_mutex_lock(&heap_lock);

    Block* block = (Block*)((char*)ptr - sizeof(Block));
    block->is_free = 1;

    // Merge adjacent free blocks
    Block* current = free_list;
    while (current && current->next) {
        if ((char*)current + sizeof(Block) + current->size == (char*)current->next) {
            current->size += sizeof(Block) + current->next->size;
            current->next = current->next->next;
        } else {
            current = current->next;
        }
    }

    pthread_mutex_unlock(&heap_lock);
}

void* heap_realloc(void* ptr, size_t size) {
    if (!ptr) {
        return heap_malloc(size);
    }

    if (size == 0) {
        heap_free(ptr);
        return NULL;
    }

    pthread_mutex_lock(&heap_lock);

    Block* block = (Block*)((char*)ptr - sizeof(Block));
    if (block->size >= size) {
        pthread_mutex_unlock(&heap_lock);
        return ptr;
    }

    void* new_ptr = heap_malloc(size);
    if (new_ptr) {
        memcpy(new_ptr, ptr, block->size);
        heap_free(ptr);
    }

    pthread_mutex_unlock(&heap_lock);
    return new_ptr;
}
