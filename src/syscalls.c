#include "fd.h"
#include "io.h"
#include "sys.h"
#include "idt.h"
#include "sh.h"
#include "mem.h"
#include <stdint.h>
#include <sys/stat.h>
#include <string.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/times.h>
#include <errno.h>
#include <stdio.h>
#include "api.h"
#include "syscall_nums.h"

#define STDIN_BUF_LEN 256

#ifndef S_IFCHR
#define S_IFCHR 0020000
#endif

#ifdef errno
#undef errno
#endif


struct InterruptFrame {
    long ebx;
    long ecx;
    long edx;
    long esi;
    long edi;
    long ebp;
    long eax;
    int  xds;
    int  xes;
    int  xfs;
    int  xgs;
    
    // Pushed by entry stub to save original EAX / syscall number
    long orig_eax;

    long eip;
    int  xcs;
    long eflags;
    long esp;
    int  xss;
};

typedef int (*Syscall)(struct InterruptFrame *);

#define SYSCALL_DEFINE0(fn) int sys_##fn(struct InterruptFrame *regs)

#define SYSCALL_DEFINE1(fn, t1, a1) \
    static int do_sys_##fn(t1 a1); \
    int sys_##fn(struct InterruptFrame *regs) { \
        return do_sys_##fn((t1)regs->ebx); \
    } \
    static int do_sys_##fn(t1 a1)

#define SYSCALL_DEFINE2(fn, t1, a1, t2, a2) \
    static int do_sys_##fn(t1 a1, t2 a2); \
    int sys_##fn(struct InterruptFrame *regs) { \
        return do_sys_##fn((t1)regs->ebx, (t2)regs->ecx); \
    } \
    static int do_sys_##fn(t1 a1, t2 a2)

#define SYSCALL_DEFINE3(fn, t1, a1, t2, a2, t3, a3) \
    static int do_sys_##fn(t1 a1, t2 a2, t3 a3); \
    int sys_##fn(struct InterruptFrame *regs) { \
        return do_sys_##fn((t1)regs->ebx, (t2)regs->ecx, (t3)regs->edx); \
    } \
    static int do_sys_##fn(t1 a1, t2 a2, t3 a3)


static char stdinBuf[STDIN_BUF_LEN];
static size_t stdinBufLen = 0;
static size_t stdinBufPos = 0;
static int stdinLineReady = 0; // whether the stdin buffer is ready to be read (newline has been received)

static void readStdinLine(void) {
    int pos = 0;
    for (;;) {
        char c;
        while ((c = getInput()) == 0);
        if (c == '\n' || c == '\r') {
            stdinBuf[pos++] = '\n';
            writeChar('\n'); // echo newline
            break;
        }
        else if (c == '\b') {
            if (pos > 0) {
                pos--;
                writeChar('\b');
            }
        }
        else {
            stdinBuf[pos] = c;
            pos++;
            writeChar(c);
        }
    }
    stdinLineReady = 1;
    stdinBufLen = pos; // excludes null term and newline
    stdinBuf[pos] = '\0';
    stdinBufPos = 0;
}

// put function for stdout/stderr
static int __stdout_put(char c, struct __file *f) {
    (void)f;
    writeChar(c);
    return 0;
}

// Internal read function so __stdin_get and sys_read share implementation
static int kread_impl(int fd, void* buf, size_t count) {
    if (fd == 0) {
        if (!stdinLineReady) {
            readStdinLine();
        }
        size_t copyCount = count;
        size_t actualCount = stdinBufLen - stdinBufPos; // number of characters in the buffer we want to read
        if (copyCount > actualCount) copyCount = actualCount; // prevents overflow
        memcpy(buf, &stdinBuf[stdinBufPos], copyCount);
        stdinBufPos += copyCount;
        if (stdinBufPos >= stdinBufLen) stdinLineReady = 0; // stdin buffer done reading
        return (int)copyCount;
    }
    return read(fd, buf, count);
}

static int __stdin_get(struct __file *f) {
    (void)f;
    unsigned char c;
    if (kread_impl(0, &c, 1) == 1) {
        return (int)c;
    } else {
        return EOF;
    }
}

// stdout FILE - write only, no buffering flags needed
static FILE __stdout = {
    .flags = __SWR,// write only
    .put   = __stdout_put,
    .get   = NULL,
    .flush = NULL,
};

// stdin FILE - read only
static FILE __stdin = {
    .flags = __SRD, // read-only
    .put   = NULL,
    .get   = __stdin_get,
    .flush = NULL,
};

// stderr - reuse stdout's put
static FILE __stderr = {
    .flags = __SWR,
    .put   = __stdout_put,
    .get   = NULL,
    .flush = NULL,
};

// The library declares these as 'FILE * const'
FILE * const stdout = &__stdout;
FILE * const stdin  = &__stdin;
FILE * const stderr = &__stderr;


SYSCALL_DEFINE3(open, const char*, path, int, flags, int, mode) {
    return open(path, flags, mode);
}

SYSCALL_DEFINE1(close, int, fd) {
    return close(fd);
}

SYSCALL_DEFINE3(read, int, fd, void*, buf, size_t, count) {
    return kread_impl(fd, buf, count);
}

SYSCALL_DEFINE3(write, int, fd, const void*, buf, size_t, count) {
    //fmtWrite("sys_write called, fd=%d count=%d\n", fd, count);
    return write(fd, buf, count);
}

SYSCALL_DEFINE3(lseek, int, fd, off_t, offset, int, whence) {
    errno = ESPIPE;   // not seekable
    return (off_t)-1;
}

SYSCALL_DEFINE2(fstat, int, fd, struct stat*, st) {
    memset(st, 0, sizeof(struct stat));
    st->st_mode = S_IFCHR;
    st->st_size = 0;
    st->st_blksize = 0;
    st->st_blocks = 0;
    st->st_dev = 0;
    st->st_ino = 0;
    st->st_nlink = 1;
    st->st_uid = 0;
    st->st_gid = 0;
    st->st_rdev = 0;
    return 0;
}

SYSCALL_DEFINE1(isatty, int, fd) {
    return (fd == 0 || fd == 1 || fd == 2) ? 1 : 0;
}

SYSCALL_DEFINE0(getpid) {
    return 1;
}

SYSCALL_DEFINE2(kill, int, pid, int, sig) {
    return -1;
}

SYSCALL_DEFINE1(exit, int, status) {
    //halt();
    sh(NULL, NULL);
    return 0;
}

SYSCALL_DEFINE1(sbrk, int, increment) {
    return (int)(uintptr_t)kbrk(increment);
}

SYSCALL_DEFINE1(times, struct tms*, buf) { 
    errno = ENOSYS; 
    return -1; 
}

SYSCALL_DEFINE1(wait, int*, status) { 
    errno = ENOSYS; 
    return -1; 
}

SYSCALL_DEFINE2(link, const char*, old, const char*, new) { 
    errno = ENOSYS; 
    return -1; 
}

SYSCALL_DEFINE1(unlink, const char*, name) { 
    errno = ENOSYS; 
    return -1; 
}

SYSCALL_DEFINE2(stat, const char*, file, struct stat*, st) { 
    errno = ENOSYS; 
    return -1; 
}

SYSCALL_DEFINE2(gettimeofday, struct timeval*, tv, void*, tz) { 
    errno = ENOSYS; 
    return -1; 
}

SYSCALL_DEFINE0(clock) { 
    errno = ENOSYS; 
    return (clock_t)-1; 
}

const static Syscall syscallTable[] = {
    [SYS_read]  = sys_read,
    [SYS_write] = sys_write,
    [SYS_exit]  = sys_exit,
    [SYS_sbrk]  = sys_sbrk,
    [SYS_times] = sys_times,
    [SYS_wait]  = sys_wait,
    [SYS_link]  = sys_link,
    [SYS_unlink]= sys_unlink,
    [SYS_stat]  = sys_stat,
    [SYS_gettimeofday] = sys_gettimeofday,
    [SYS_clock] = sys_clock,
    [SYS_open]  = sys_open,
    [SYS_close] = sys_close,
    [SYS_fstat] = sys_fstat,
    [SYS_isatty]= sys_isatty,
    [SYS_getpid]= sys_getpid,
    [SYS_kill]  = sys_kill,
    [SYS_lseek] = sys_lseek,
};
#define SYSCALL_COUNT (sizeof(syscallTable)/sizeof(*syscallTable))

int syscallDispatcher(struct InterruptFrame *regs) {
    //fmtWrite("syscall nr=%d\n", regs->orig_eax);
    unsigned int nr = (unsigned int)regs->orig_eax;
    if (nr >= SYSCALL_COUNT || !syscallTable[nr]) {
        fmtWrite("Unhandled syscall: %d\n", regs->orig_eax);
        //userPanic();
        return -ENOSYS;
        //__asm__ volatile("int $0x3");
    }
    return syscallTable[nr](regs);
}