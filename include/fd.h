#ifndef LANTERN_FD_H
#define LANTERN_FD_H

#define O_RDONLY 0
#define O_WRONLY 1
#define O_RDWR   2
#define O_CREAT  4
#define O_TRUNC  8
#define O_APPEND 16

#define FD_MAX_FILES 8

#include <stdint.h>
#include <stddef.h>

struct FileDesc {
    char     name[100];
    int      isInUse;
    int      dirty;
    uint8_t  *data;
    uint32_t size;
    uint32_t rwPos;
    int      flags; // 0 = read, 1 = write, 2 = read/write
};

void fdInit(void);
int open(const char* path, int flags, unsigned mode);
int read(int fd_, void *buf, size_t count);
int write(int fd_, const void *buf, size_t count);
int close(int fd_);

#endif // LANTERN_FD_H