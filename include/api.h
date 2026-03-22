#ifndef LANTERN_API_H
#define LANTERN_API_H

struct MemoryInfo;

struct KernelAPI {
    void (*fmtWrite)(const char *fmt, ...);
    void (*fmtGet)(const char *fmt, ...);
    void (*sfmtWrite)(char *buf, const char *fmt, ...);
    void (*clearScreen)(void);

    int  (*strcmp)(const char *a, const char *b);
    int  (*atoi)(const char *s);

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


#ifndef KERN
volatile unsigned int *sysTicks;
#define fmtWrite      k->fmtWrite
#define fmtGet        k->fmtGet
#define sfmtWrite     k->sfmtWrite
#define clearScreen   k->clearScreen
#define strcmp        k->strcmp
#define atoi          k->atoi
#define reboot        k->reboot
#define shutdown      k->shutdown
#define userPanic     k->userPanic
#define sendInterrupt k->sendInterrupt
#define sysTicks      (*k->sysTicks)
#define getTotalMem   k->getTotalMem
#define getUsedMem    k->getUsedMem
#define memInfo       k->memInfo
#define tarLoad       k->tarLoad
#define tarList       k->tarList
#define tarPrintFile  k->tarPrintFile
#define tarReadFile   k->tarReadFile
#endif

#endif