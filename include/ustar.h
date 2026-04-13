#ifndef LANTERN_TAR_H
#define LANTERN_TAR_H

#include "kstdint.h"

#define TAR_MAX_SECTORS 128  // 64 kb
#define TAR_BUF_SIZE (TAR_MAX_SECTORS * 512)

#define TAR_FILE   '0'
#define TAR_HLINK  '1'
#define TAR_SLINK  '2'
#define TAR_CDEV   '3'
#define TAR_BDEV   '4'
#define TAR_DIR    '5'
#define TAR_PIPE   '6'

extern uint8_t tarBuf[TAR_BUF_SIZE];

struct TarHeader {
    char name[100];
    char mode[8];
    char ownerID[8];
    char groupID[8];
    char size[12]; // octal
    char lastModTime[12]; // octal
    char checksum[8];
    char type;
    char linkedFileName[100];
    char ustar[6]; // should be "ustar\0"
    char ustarVer[2]; // should be "00" (?)
    char ownerName[32];
    char ownerGroup[32];
    char majNum[8];
    char minNum[8];
    char prefix[155];
    char _[12]; // padding
};

int tarRead(uint8_t *buf, char *fname, char **dataPtr);
int tarReadFile(const char *fname, char **out);
void tarPrintFile(const char *fname);
void tarList(const char* flag);
void tarLoad(void);
struct TarHeader *tarNext(struct TarHeader *th);
int tarValid(struct TarHeader *th);
void tarFlush(void);
int tarRm(const char *fname);
int tarTouch(const char *fname);
int tarEdit(const char *fname, const char *data, unsigned int size);

#endif // LANTERN_TAR_H