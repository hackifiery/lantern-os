#ifndef LANTERN_SYS_H
#define LANTERN_SYS_H

struct multibootInfo {
    unsigned int flags;
    unsigned int mem_lower;
    unsigned int mem_upper;
    unsigned int boot_device;
    unsigned int cmdline;
    unsigned int mods_count;
    unsigned int mods_addr;
    // tba
} __attribute__((packed));

// note: in kb
unsigned int getUsedMem(void);
// note: in kb
unsigned int getTotalMem(struct multibootInfo* mbPtr);
void reboot(void);
void shutdown(void);

#endif // LANTERN_SYS_H