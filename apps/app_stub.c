#include <stdio.h>
#include <errno.h>
#include <stdint.h>
#include <time.h>
#include <string.h>
#include <sys/stat.h>
#include <string.h>
#include "api.h"
#ifdef errno
#undef errno
#endif
int errno = 0;

// Global pointer to the kernel API (set by the app's main)
struct KernelAPI *g_api = NULL;

// ---------- stdout / stdin / stderr ----------
static int __stdout_put(char c, struct __file *f) {
    (void)f;
    if (g_api && g_api->writeChar) g_api->writeChar(c);
    return 0;
}

static int __stdin_get(struct __file *f) {
    (void)f;
    // Not implemented – return EOF
    return EOF;
}

static FILE __stdout = {
    .flags = __SWR,
    .put   = __stdout_put,
    .get   = NULL,
    .flush = NULL,
};
static FILE __stdin = {
    .flags = __SRD,
    .put   = NULL,
    .get   = __stdin_get,
    .flush = NULL,
};
static FILE __stderr = {
    .flags = __SWR,
    .put   = __stdout_put,
    .get   = NULL,
    .flush = NULL,
};

FILE * const stdout = &__stdout;
FILE * const stdin  = &__stdin;
FILE * const stderr = &__stderr;

// ---------- heap ----------
#define HEAP_SIZE (1024 * 1024)
static uint8_t heap_memory[HEAP_SIZE];
static uintptr_t heap_brk = (uintptr_t)heap_memory;

void *_sbrk(int incr) {
    uintptr_t old = heap_brk;
    if (incr == 0) return (void*)old;
    if (heap_brk + incr > (uintptr_t)heap_memory + HEAP_SIZE) {
        errno = ENOMEM;
        return (void*)-1;
    }
    heap_brk += incr;
    return (void*)old;
}

// ---------- syscall stubs ----------
int _close(int fd)          { return -1; }
int _fstat(int fd, struct stat *st) {
    memset(st, 0, sizeof(*st));
    st->st_mode = S_IFCHR;
    return 0;
}
int _isatty(int fd)         { return (fd == 0 || fd == 1 || fd == 2); }
off_t _lseek(int fd, off_t off, int whence) { errno = ESPIPE; return -1; }
int _read(int fd, void *buf, size_t count) { return -1; }
int _write(int fd, const void *buf, size_t count) {
    if (fd == 1 || fd == 2) {
        const char *p = buf;
        for (size_t i = 0; i < count; i++) {
            if (g_api && g_api->writeChar) g_api->writeChar(p[i]);
        }
        return count;
    }
    errno = EBADF;
    return -1;
}
int _getpid(void)           { return 1; }
int _kill(int pid, int sig) { errno = ENOSYS; return -1; }
void _exit(int status)      { while (1) __asm__("hlt"); }
int _times(struct tms *buf) { errno = ENOSYS; return -1; }
int _wait(int *status)      { errno = ENOSYS; return -1; }
int _link(const char *old, const char *new) { errno = ENOSYS; return -1; }
int _unlink(const char *name) { errno = ENOSYS; return -1; }
int _stat(const char *file, struct stat *st) { errno = ENOSYS; return -1; }
int _gettimeofday(struct timeval *tv, void *tz) { errno = ENOSYS; return -1; }
clock_t _clock(void)        { errno = ENOSYS; return (clock_t)-1; }