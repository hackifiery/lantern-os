#include "tar.h"
#include "ata.h"
#include "string_utils.h"
#include "io.h"

#define TAR_MAX_SECTORS 64  // 32 kb
#define TAR_START_LBA 101
#define ceilDiv(x, y) (x / y + (x % y != 0))

static unsigned short tarBuf[TAR_MAX_SECTORS * 256];
//static char* cwd = "/";

static int oct2bin(unsigned char *str, int size) {
    int n = 0;
    unsigned char *c = str;
    while (size-- > 0) {
        n *= 8;
        n += *c - '0';
        c++;
    }
    return n;
}

static int memcmp(const void *a, const void *b, unsigned int n) {
    const unsigned char *pa = (const unsigned char *)a;
    const unsigned char *pb = (const unsigned char *)b;
    for (unsigned int i = 0; i < n; i++) {
        if (pa[i] != pb[i]) return pa[i] - pb[i];
    }
    return 0;
}

static int tarValid(struct TarHeader *th) {
    if (memcmp(th->ustar, "ustar", 5) == 0) return 1;
    else return 0;
}

// note: tarNext DOES NOT MODIFY BUF!!! SET IT TO IT!!!
static struct TarHeader *tarNext(struct TarHeader *th) {
    int size = oct2bin((unsigned char *)th->size, 11);
    int blocks = ceilDiv(size, 512) + 1; // # of sectors to advance to the next file
    return (struct TarHeader*)((unsigned char *)th + (blocks * 512));
}

static struct TarHeader *tarFind(unsigned short *buf, char *fname) {
    struct TarHeader *curr = (struct TarHeader*) buf;
    while (tarValid(curr)) {
        if (strcmp(curr->name, fname) == 0) return curr;
        curr = tarNext(curr);
    }
    return 0;
}

// returns file size
int tarRead(unsigned short *buf, char *fname, char **dataPtr) {
    struct TarHeader *th = tarFind(buf, fname);
    if (!th) return 0;
    *dataPtr = (char*)th + 512;
    return oct2bin(th->size, 11);
}

void tarList(void) {
    struct TarHeader *curr = (struct TarHeader *)tarBuf;
    while (tarValid(curr)){
        switch (curr->type) {
            case TAR_FILE: fmtWrite("[file] %s\n", curr->name); break;
            case TAR_DIR:  fmtWrite("[dir]  %s\n", curr->name); break;
            default:       fmtWrite("[????] %s\n", curr->name); break;
        }
        curr = tarNext(curr);
    }
}

void tarPrintFile(const char *fname) {
    char *data;
    const unsigned int size = tarRead(tarBuf, fname, &data);
    if (!size) {fmtWrite("%s not found\n", fname); return;}
    for (int i = 0; i < size; i++) fmtWrite("%c", data[i]); // safer than directly printing data
}

void tarLoad(void) {
    for (unsigned int i = 0; i < TAR_MAX_SECTORS; i++) ataRead(TAR_START_LBA + i, tarBuf + (i * 256));
}
