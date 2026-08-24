#include "api.h"
extern int main(/*struct KernelAPI *api*/void);
extern void _exit(int status);
//struct KernelAPI *g_api;

void _start(struct KernelAPI *api) {
    //g_api = api;
    int res = main(/*api*/);
    _exit(res);
}