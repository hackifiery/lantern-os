#include "fd.h"
#include "io.h"
#include "sys.h"
#include "mem.h"
#include <stdint.h>
#include <sys/stat.h>
#include <string.h>
#include <sys/types.h>
#include <errno.h>
#include <stdio.h>

#define STDIN_BUF_LEN 256

#ifndef S_IFCHR
#define S_IFCHR 0020000
#endif
#undef errno
//extern void writeChar(char c);
//extern char getInput(void);

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

// get function for stdin (now replaced with __read)
/*static int __stdin_get(struct __file *f) {
    (void)f;
    char c;
    while ((c = getInput()) == 0) {
        // wait for keypress
    }
    writeChar(c);  // echo
    if (c != '\b') return (unsigned char)c;
    else return 0;  // don't return backspace to caller
}*/
int _read(int fd, void* buf, size_t count);
static int __stdin_get(struct __file *f) {
    (void)f;
    unsigned char c;
    if (_read(0, &c, 1) == 1) {
        return (int)c;
    } else {
        return EOF;
    }
}
/*static int __stdin_get(struct __file *f) {
    char c;
    while ((c = getInput()) == 0) {}
    fmtWrite("DEBUG: got %d ('%c')\n", c, c);   // kernel debug
    writeChar(c);   // echo
    return c;
}*/

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

int errno = 0;

int _open(const char* path, int flags, int mode) {
    return open(path, flags, mode);
}

int _close(int fd) {
    return close(fd);
}

int _read(int fd, void* buf, size_t count) {
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

int _write(int fd, const void* buf, size_t count) {
    return write(fd, buf, count);
}

off_t _lseek(int fd, off_t offset, int whence) {
    errno = ESPIPE;   // not seekable
    return (off_t)-1;
}

int lseek(int fd, int offset, int whence) {
    return _lseek(fd, offset, whence);
}

int _fstat(int fd, struct stat* st) {
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

int fstat(int fd, struct stat* st) {
    return _fstat(fd, st);
}

int _isatty(int fd) {
    return (fd == 0 || fd == 1 || fd == 2) ? 1 : 0;
}

int isatty(int fd) {
    return _isatty(fd);
}

int _getpid(void) {
    return 1;
}

int getpid(void) {
    return _getpid();
}

int _kill(int pid, int sig) {
    return -1;
}

int kill(int pid, int sig) {
    return _kill(pid, sig);
}

void _exit(int status) {
    halt();
}

void* _sbrk(int increment) {
    return kbrk(increment);
}

void* sbrk(int increment) {
    return _sbrk(increment);
}
int _times(struct tms* buf)    { errno = ENOSYS; return -1; }
int _wait(int* status)         { errno = ENOSYS; return -1; }
int _link(const char* old, const char* new) { errno = ENOSYS; return -1; }
int _unlink(const char* name)  { errno = ENOSYS; return -1; }
int _stat(const char* file, struct stat* st) { errno = ENOSYS; return -1; }
int _gettimeofday(struct timeval* tv, void* tz) { errno = ENOSYS; return -1; }
clock_t _clock(void)           { errno = ENOSYS; return (clock_t)-1; }
