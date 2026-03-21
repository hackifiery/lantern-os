#ifndef LANTERN_API_H
#define LANTERN_API_H

struct KernelAPI {
    void (*fmtWrite)(const char *fmt, ...);
    void (*fmtGet)(const char *fmt, ...);
    void (*sfmtWrite)(char *buf, const char *fmt, ...);
    int  (*strcmp)(const char *a, const char *b);
    int  (*atoi)(const char *s);
};

#endif