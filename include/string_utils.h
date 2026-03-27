#ifndef LANTERN_STRING_UTILS_H
#define LANTERN_STRING_UTILS_H

#include "kstdint.h"

int strcmp(const char *s1, const char *s2);
void lower(char* str);
void upper(char* str);
unsigned int strlen(const char *s);

#endif // LANTERN_STRING_UTILS_H