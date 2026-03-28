#include "ustar.h"
#include "ata.h"
#include "string_utils.h"
#include "io.h"
#include "kstdint.h"

#define TAR_MAX_SECTORS 64  // 32 kb
#define TAR_BUF_SIZE (TAR_MAX_SECTORS * 512)
#define TAR_START_LBA 101
#define ceilDiv(x, y) (x / y + (x % y != 0))

static uint8_t tarBuf[TAR_BUF_SIZE];

typedef unsigned long ul;
//static char* cwd = "/";

static struct Datetime{
    int year, month, day, hour, minute, second;
};

static void unix2date(ul seconds, struct Datetime *dt) {
    // seconds into time of day
    ul days = seconds / 86400;
    ul rem = seconds % 86400;
    
    dt->hour = rem / 3600;
    dt->minute = (rem % 3600) / 60;
    dt->second = rem % 60;

    // year
    int year = 1970;
    while (1) {
        int leap = (year % 4 == 0 && (year % 100 != 0 || year % 400 == 0));
        int days_in_year = leap ? 366 : 365;
        if (days < days_in_year) break;
        days -= days_in_year;
        year++;
    }
    dt->year = year;

    // month
    static const int days_per_month[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    int leap = (year % 4 == 0 && (year % 100 != 0 || year % 400 == 0));
    
    int month = 0;
    for (month = 0; month < 12; month++) {
        int dim = days_per_month[month];
        if (month == 1 && leap) dim = 29; // leap yr
        if (days < dim) break;
        days -= dim;
    }
    dt->month = month + 1; // 1-12
    dt->day = (int)days + 1;    // 1-31
}

static int oct2bin(uint8_t *str, int size) {
    int n = 0;
    uint8_t *c = str;
    while (size-- > 0) {
        n *= 8;
        n += *c - '0';
        c++;
    }
    return n;
}

static int memcmp(const void *a, const void *b, unsigned int n) {
    const uint8_t *pa = (const uint8_t *)a;
    const uint8_t *pb = (const uint8_t *)b;
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
    int size = oct2bin((uint8_t *)th->size, 11);
    int blocks = ceilDiv(size, 512) + 1; // # of sectors to advance to the next file
    return (struct TarHeader*)((uint8_t *)th + (blocks * 512));
}

static struct TarHeader *tarFind(uint8_t *buf, const char *fname) {
    struct TarHeader *curr = (struct TarHeader*) buf;
    while (tarValid(curr)) {
        if (strcmp(curr->name, fname) == 0) return curr;
        curr = tarNext(curr);
    }
    return 0;
}

// returns file size
int tarRead(uint8_t *buf, char *fname, char **dataPtr) {
    struct TarHeader *th = tarFind(buf, fname);
    if (!th) return 0;
    *dataPtr = (char*)th + 512;
    return oct2bin(th->size, 11);
}

void tarList(const char* flag) {
    struct TarHeader *curr = (struct TarHeader *)tarBuf;
    //fmtWrite("Directory listing of /\n");
    if (strcmp(flag, "-l") == 0) {
        fmtWrite("type  size         lastModify          name\n");
        fmtWrite("===== ========= ================== ==================");
    }
    while (tarValid(curr)){
        if (strcmp(flag, "-l") == 0) {
            fmtWrite("\n");
            switch (curr->type) {
                case TAR_FILE: fmtWrite("file"); break;
                case TAR_DIR:  fmtWrite("dir "); break;
                default:       fmtWrite("????"); break;
            }
            {unsigned long unixDate = oct2bin(curr->lastModTime, 11); struct Datetime dt; unix2date(unixDate, &dt);
            fmtWrite("   %8d %02d-%02d-%04d@%02d:%02d:%02d %s", oct2bin(curr->size, 11), dt.month, dt.day, dt.year, dt.hour, dt.minute, dt.second, curr->name);
            }
        }
        else {
            switch (curr->type) {
                case TAR_FILE: fmtWrite("%s  ", curr->name); break;
                case TAR_DIR:  fmtWrite("%s/  ", curr->name); break;
                default:       fmtWrite("%s  ", curr->name); break;
            }
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
    for (unsigned int i = 0; i < TAR_MAX_SECTORS; i++) ataRead(TAR_START_LBA + i, (uint16_t *)(tarBuf + (i * 512)));
}

void tarFlush(void) {
    //fmtWrite("after rm, first entry: %s\n", ((struct TarHeader*)tarBuf)->name);
    for (unsigned int i = 0; i < TAR_MAX_SECTORS; i++) ataWrite(TAR_START_LBA + i, (uint16_t *)(tarBuf + (i * 512)));
}

int tarReadFile(const char *fname, char **out) {
    return tarRead(tarBuf, (char *)fname, out);
}

int tarRm(const char *fname) {
    {char *tmp;
    if (!tarReadFile(fname, &tmp)) return 0;}

    struct TarHeader *toDel = tarFind(tarBuf, fname);
    int fSize    = oct2bin((uint8_t *)toDel->size, 11);
    int delBytes = (ceilDiv(fSize, 512) + 1) * 512;

    uint8_t *buf      = tarBuf;
    uint8_t *start    = (uint8_t *)toDel;
    int      bufBytes = TAR_BUF_SIZE;
    int      startIdx = start - buf;

    for (int i = startIdx; i < bufBytes - delBytes; i++)
        buf[i] = buf[i + delBytes];

    for (int i = bufBytes - delBytes; i < bufBytes; i++)
        buf[i] = 0;

    return 1;
}