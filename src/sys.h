#ifndef LANTERN_SYS_H
#define LANTERN_SYS_H

/*struct multibootInfo {
    unsigned int flags;
    unsigned int mem_lower;
    unsigned int mem_upper;
    unsigned int boot_device;
    unsigned int cmdline;
    unsigned int mods_count;
    unsigned int mods_addr;
    // tba
} __attribute__((packed));*/

struct Registers {
    unsigned int edi, esi, ebp, esp, ebx, edx, ecx, eax;
    unsigned int int_no, err_code;
    unsigned int eip, cs, eflags;
};

struct E820Entry {
    unsigned long long base;
    unsigned long long length;
    unsigned int type;
    unsigned int extended;
} __attribute__((packed));

struct MemoryInfo {
    unsigned int entry_count;
    struct E820Entry* entries;
};

// note: in kb
unsigned int getUsedMem(void);
// note: in kb
unsigned int getTotalMem(struct MemoryInfo* mem);
void reboot(void);
void shutdown(void);

extern void halt(void);


#endif // LANTERN_SYS_H