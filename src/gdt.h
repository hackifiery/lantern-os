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