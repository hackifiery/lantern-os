#include <stdio.h>
#include "api.h"

extern struct KernelAPI *g_api;

void main(struct KernelAPI *api) {
    g_api = api;
    printf("hi\n");
}