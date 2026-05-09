#include "fd.h"
#include "io.h"
#include "sys.h"
#include <stdint.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>

#ifndef S_IFCHR
#define S_IFCHR 0020000
#endif

int _open(const char* path, int flags, int mode) {
    return open(path, flags, mode);
}

int _close(int fd) {
    return close(fd);
}

int _read(int fd, void* buf, size_t count) {
    return read(fd, buf, count);
}

int _write(int fd, const void* buf, size_t count) {
    return write(fd, buf, count);
}

int _lseek(int fd, int offset, int whence) {
    return -1;  // not implemented
}

int lseek(int fd, int offset, int whence) {
    return _lseek(fd, offset, whence);
}

int _fstat(int fd, struct stat* st) {
    st->st_mode = S_IFCHR;  // pretend everything is a character device
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
    return getpid();
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
    // heap not implemented yet — return -1
    return (void*)-1;
}