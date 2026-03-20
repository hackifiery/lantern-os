#include "tar.h"
#include "ata.h"
#include "string_utils.h"
#include "io.h"

#define TAR_MAX_SECTORS 64  // 32 kb
#define TAR_START_LBA 0

static unsigned short tarBuf[TAR_MAX_SECTORS * 256];

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

void tarDebug(void) {
    unsigned char *ptr = (unsigned char *)tarBuf;
    fmtWrite("magic: ");
    for (int i = 0; i < 6; i++) fmtWrite("%02x ", ptr[257 + i]);
    fmtWrite("\n");
    int entry = 0;
    while (memcmp(ptr + 257, "ustar", 5) == 0) {
        int filesize = oct2bin(ptr + 0x7c, 11);
        fmtWrite("entry %d: '%s' (%d bytes)\n", entry++, (char*)ptr, filesize);
        ptr += (((filesize + 511) / 512) + 1) * 512;
    }
    fmtWrite("done\n");
}

int tarRead(unsigned char *ar, char *name, char **out) {
    unsigned char *ptr = ar;

    while (!memcmp(ptr + 257, "ustar", 5)) {
        int filesize = oct2bin(ptr + 0x7c, 11);
        if (!memcmp(ptr, name, strlen(name) + 1)) {
            *out = ptr + 512;
            return filesize;
        }
        ptr += (((filesize + 511) / 512) + 1) * 512;
    }
    return 0;
}

void tarLoad(void) {
    for (unsigned int i = 0; i < TAR_MAX_SECTORS; i++) {
        ataRead(TAR_START_LBA + i, tarBuf + (i * 256));
    }
}

void tarPrintFile(const char *name) {
    char *out = 0;
    int size = tarRead((unsigned char *)tarBuf, (char *)name, &out);

    if (size == 0) {
        fmtWrite("file not found: %s\n", name);
        return;
    }

    //fmtWrite("=== %s (%d bytes) ===\n", name, size);
    for (int i = 0; i < size; i++) {
        fmtWrite("%c", out[i]);
    }
    fmtWrite("\n");
}

void tarList(void) {
    unsigned char *ptr = (unsigned char *)tarBuf;
    while (memcmp(ptr + 257, "ustar", 5) == 0) {
        int filesize = oct2bin(ptr + 0x7c, 11);
        char type = ptr[156];
        char typeChar = (type == '5') ? 'd' : 'f';
        fmtWrite("[%c] %s (%d bytes)\n", typeChar, (char*)ptr, filesize);
        ptr += (((filesize + 511) / 512) + 1) * 512;
    }
}