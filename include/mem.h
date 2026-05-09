#ifndef LANTERN_MEM_H
#define LANTERN_MEM_H

#include <stdint.h>
#include "sys.h"
#include <stddef.h>
#define BLOCK_HEADER_SIZE sizeof(struct MemoryBlock)
#define MIN_BLOCK_SIZE 16

struct HeapState {
    uintptr_t next; // next free block
    uintptr_t end; // DO NOT MODIFY AFTER INIT!!!!
};

void memInit(void* start, const size_t size);

#endif // LANTERN_MEM_H