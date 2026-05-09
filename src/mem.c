#include "mem.h"
#include <stdint.h>
#include <stddef.h>

static struct HeapState* heap = NULL;
static uint32_t heapTotal = 0; // in bytes
static uint32_t heapUsed = 0;  // in bytes
static int initialized = 0;

void memInit(void* start, const size_t size) {
    heap = (struct HeapState*)start;
    heap->next = (uintptr_t)start + sizeof(struct HeapState);
    heap->end = (uintptr_t)(start + size);
    heapTotal = size;
    heapUsed = 0;
    initialized = 1;
}

void* kmalloc(const size_t size) {
    if (!initialized || !heap) return NULL;
    uintptr_t oldNext = heap->next;
    uintptr_t next = heap->next + size;
    if (next > heap->end) return NULL;
    heap->next = next;
    uintptr_t start = (uintptr_t)heap + sizeof(struct HeapState);
    heapUsed = heap->next - start;
    return (void *)(oldNext);
}

void* kfree(void *block) {
    return NULL; // TODO: implement better allocator
}

void* sbrk(int increment) {
    if (!initialized) return (void *)-1;
    uintptr_t old = heap->next;
    if (heap->next + increment > heap->end) return (void *)-1;
    heap->next += increment;
    return (void *)old;
}