#ifndef LANTERN_STDINT_H
#define LANTERN_STDINT_H
#include "kstdint.h"

// exact width types
typedef unsigned char       uint8_t;
typedef unsigned short      uint16_t;
typedef unsigned int        uint32_t;
typedef unsigned long long  uint64_t;

typedef signed char         int8_t;
typedef signed short        int16_t;
typedef signed int          int32_t;
typedef signed long long    int64_t;

// pointer-sized types
typedef unsigned int        uintptr_t;
typedef signed int          intptr_t;

// max width types
typedef unsigned long long  uintmax_t;
typedef signed long long    intmax_t;

// size type
typedef unsigned int        size_t;
typedef signed int          ssize_t;

// limits
#define UINT8_MAX   0xFF
#define UINT16_MAX  0xFFFF
#define UINT32_MAX  0xFFFFFFFF
#define UINT64_MAX  0xFFFFFFFFFFFFFFFFULL

#define INT8_MAX    0x7F
#define INT16_MAX   0x7FFF
#define INT32_MAX   0x7FFFFFFF
#define INT64_MAX   0x7FFFFFFFFFFFFFFFLL

#define INT8_MIN    (-0x7F - 1)
#define INT16_MIN   (-0x7FFF - 1)
#define INT32_MIN   (-0x7FFFFFFF - 1)
#define INT64_MIN   (-0x7FFFFFFFFFFFFFFFLL - 1)

#define SIZE_MAX    UINT32_MAX
#define UINTPTR_MAX UINT32_MAX

// null
#ifndef NULL
#define NULL ((void*)0)
#endif

#endif // LANTERN_STDINT_H