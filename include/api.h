#ifndef LANTERN_API_H
#define LANTERN_API_H

struct MemoryInfo;

struct KernelAPI {
    void (*fmtWrite)(const char *fmt, ...);
    void (*fmtGet)(const char *fmt, ...);
    void (*sfmtWrite)(char *buf, const char *fmt, ...);
    void (*writeChar)(char c);
    void (*clearScreen)(void);

    void (*reboot)(void);
    void (*shutdown)(void);
    void (*userPanic)(void);
    void (*sendInterrupt)(int n);
    unsigned int (*sysTicks)(void);  // can't share volatile var directly

    unsigned int (*getTotalMem)(struct MemoryInfo *mb);
    unsigned int (*getUsedMem)(void);

    void (*tarLoad)(void);
    void (*tarList)(void);
    void (*tarPrintFile)(const char *name);
    int  (*tarReadFile)(const char *name, char **out);

    struct MemoryInfo *memInfo;  // pointer to kernel's mem info
};
#endif