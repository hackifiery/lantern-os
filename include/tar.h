#ifndef LANTERN_TAR_H
#define LANTERN_TAR_H


#define TAR_NORMAL 0
#define TAR_HLINK  1
#define TAR_SLINK  2
#define TAR_CDEV   3
#define TAR_BDEV   4
#define TAR_DIR    5
#define TAR_PIPE   6

// TODO: use this struct for all tar-related functions
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
    char ustarVer[2]; // should be "00"
    char ownerName[32];
    char ownerGroup[32];
    char majNum[8];
    char minNum[8];
    char prefix[155];
    char _[12]; // padding
};

int tarRead(unsigned char *ar, char* name, char **out);
void tarLoad(void);
void tarPrintFile(const char *name);
void tarDebug(void);
void tarList(void);
#endif // LANTERN_TAR_H