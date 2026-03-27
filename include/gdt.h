#ifndef LANTERN_GDT_H
#define LANTERN_GDT_H
#include "kstdint.h"

struct GdtEntry {
    uint16_t limitLow;
    uint16_t baseLow;
    uint8_t  baseMiddle;
    uint8_t  access;
    uint8_t  granularity;
    uint8_t  baseHigh;
} __attribute__((packed));

struct GdtPtr {
    uint16_t limit;
    uint32_t base;
} __attribute__((packed));

void initGdt(void);

#endif // LANTERN_GDT_H