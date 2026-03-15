#ifndef LANTERN_GDT_H
#define LANTERN_GDT_H

struct GdtEntry {
    unsigned short limitLow;
    unsigned short baseLow;
    unsigned char  baseMiddle;
    unsigned char  access;
    unsigned char  granularity;
    unsigned char  baseHigh;
} __attribute__((packed));

struct GdtPtr {
    unsigned short limit;
    unsigned int   base;
} __attribute__((packed));

void initGdt(void);

#endif // LANTERN_GDT_H