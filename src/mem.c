#include "mem.h"
#include "io.h"
#include "idt.h"
#include <stdint.h>
#include <stddef.h>

#define HEAP_SIZE (1024 * 1024)   // 1 MiB
static uint8_t heap_memory[HEAP_SIZE];
static uintptr_t heap_brk = (uintptr_t)heap_memory;

void* kbrk(int increment) {
    uintptr_t old = heap_brk;
    if (increment == 0) return (void*)old;
    if (heap_brk + increment > (uintptr_t)heap_memory + HEAP_SIZE) {
        return (void*)-1;   // out of memory
    }
    heap_brk += increment;
    return (void*)old;
}