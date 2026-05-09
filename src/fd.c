#include "fd.h"
#include "ustar.h"
#include <string.h>
#include <stddef.h>
//#include <sys/types.h>
#include "io.h"

struct FileDesc fdTable[FD_MAX_FILES];

void fdInit(void) {
    for (int i = 0; i < FD_MAX_FILES; i++) {
        struct FileDesc *fd = &fdTable[i];
        fd->isInUse = 0;
        fd->data = NULL;
        fd->size = 0;
        fd->rwPos = 0;
        fd->flags = 0;
        fd->name[0] = '\0';
    }
}

int open(const char* path, int flags, unsigned mode) { // mode is ignored for now
    unsigned slot = 0;
    struct TarHeader* usableTh = tarFind(path);
    if (!usableTh) { // file doesn't exist
        if (flags & O_CREAT) {
            tarTouch(path);
            usableTh = tarFind(path);
        }
        else return -1;
    }
    if (flags & O_TRUNC) {
        tarEdit(path, "", 0);
        usableTh = tarFind(path);
    }
    for (slot = 0; slot < FD_MAX_FILES; slot++) {
        if (!fdTable[slot].isInUse) goto found;
    }
    return -1; // no usable slot found
    found:
    struct FileDesc* freeFdSlot = &fdTable[slot];
    freeFdSlot->data = (uint8_t*)usableTh + sizeof(struct TarHeader); // expose data (not the header itself)
    freeFdSlot->size = tarGetSize(path);
    strcpy(freeFdSlot->name, path);
    freeFdSlot->rwPos = 0;
    freeFdSlot->flags = flags;
    freeFdSlot->dirty = 0;
    freeFdSlot->isInUse = 1;
    return slot+3; // because 0, 1, & 2 are stdin stdout, & stderr
}

int read(int fd_, void *buf, size_t count) {
    if (fd_ == 0) { // stdin
        fmtGet("%s", (char *)buf);
        return strlen((char *)buf);
        // TODO: bound stdin to count
    }
    if (fd_ == 1 || fd_ == 2) return -1;  // can't read from stdout/stderr
    if (fd_ >= FD_MAX_FILES + 3) return -1;

    struct FileDesc* fd = &(fdTable[fd_-3]);

    if (!fd->isInUse) return -1; // isInUse should be true if fd_ is from an open() call
    if (fd->rwPos >= fd->size) return 0; // eof

    if (fd->rwPos + count > fd->size) count = fd->size - fd->rwPos;
    uint8_t* src = (uint8_t*)(fd->data + fd->rwPos);
    uint8_t* dest = (uint8_t*)buf;
    for (size_t i = 0; i < count; i++) {
        dest[i] = src[i];
        fd->rwPos++;
    }
    return count;
}

int write(int fd_, const void *buf, size_t count) {
    if (fd_ == 1 || fd_ == 2) {
        for (size_t i = 0; i < count; i++) writeChar(((char*)buf)[i]);
        return count;
    }

    if (fd_ == 0) return -1;  // can't write to stdin
    if (fd_ >= FD_MAX_FILES + 3) return -1;
    struct FileDesc* fd = &(fdTable[fd_-3]);

    if (!fd->isInUse) return -1; // isInUse should be true if fd_ is from an open() call
    if (fd->rwPos >= fd->size) return 0; // eof

    if (fd->rwPos + count > fd->size) count = fd->size - fd->rwPos;
    uint8_t* dest = (uint8_t*)(fd->data + fd->rwPos);
    uint8_t* src = (uint8_t*)buf;
    for (size_t i = 0; i < count; i++) {
        dest[i] = src[i];
        fd->rwPos++;
    }
    fd->dirty = 1;
    return count;
}

int close(int fd_) {
    if (fd_ < 3) return -1; // can't close stdin/out/err
    if (fd_ >= FD_MAX_FILES + 3) return -1;
    struct FileDesc* fd = &(fdTable[fd_-3]);
    if (!fd->isInUse) return -1; // a file opened with open() should be in use
    if (fd->dirty) tarFlush();
    fd->isInUse = 0;
    fd->data = NULL;
    fd->size = 0;
    fd->rwPos = 0;
    fd->flags = 0;
    fd->name[0] = '\0';
    return 0;
}