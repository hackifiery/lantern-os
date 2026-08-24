#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/times.h>
#include <stdio.h>

#include "syscall_nums.h"

#ifdef errno
#undef errno
#endif

static inline int __syscall0(int nr) {
    int res;
    __asm__ volatile ("int $0x80" : "=a"(res) : "0"(nr) : "memory");
    return res;
}

static inline int __syscall1(int nr, long a1) {
    int res;
    __asm__ volatile ("int $0x80" : "=a"(res) : "0"(nr), "b"(a1) : "memory");
    return res;
}

static inline int __syscall2(int nr, long a1, long a2) {
    int res;
    __asm__ volatile ("int $0x80" : "=a"(res) : "0"(nr), "b"(a1), "c"(a2) : "memory");
    return res;
}

static inline int __syscall3(int nr, long a1, long a2, long a3) {
    int res;
    __asm__ volatile ("int $0x80" : "=a"(res) : "0"(nr), "b"(a1), "c"(a2), "d"(a3) : "memory");
    return res;
}

// Convert negative kernel returns to errno
static inline int _check_ret(int res) {
    if (res < 0) {
        errno = -res;
        return -1;
    }
    return res;
}


int read(int fd, void *buf, size_t count) {
    //for(;;);
    return _check_ret(__syscall3(SYS_read, fd, (long)buf, (long)count));
}
int write(int fd, const void *buf, size_t count) {
    return _check_ret(__syscall3(SYS_write, fd, (long)buf, (long)count));
}
void _exit(int status) {
    __syscall1(SYS_exit, status);
    //for (;;);
}
void *sbrk(intptr_t inc) {
    int res = __syscall1(SYS_sbrk, inc);
    if (res == -1) { errno = ENOMEM; return (void*)-1; }
    return (void*)res;
}

static int __user_stdout_put(char c, FILE *f) { (void)f; write(1, &c, 1); return 0; }
static FILE __stdout = { .flags = __SWR, .put = __user_stdout_put };
static FILE __stderr = { .flags = __SWR, .put = __user_stdout_put };
FILE * const stdout = &__stdout;
FILE * const stderr = &__stderr;