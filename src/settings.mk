ASM      := nasm
CC       := i686-elf-gcc
LD       := i686-elf-gcc          # Use GCC driver
OBJCOPY  := objcopy

LIBC_PATH   := $(HOME)/opt/cross/lib
INCLUDE_PATH := $(HOME)/opt/cross/include

# Linker flags (no libraries here)
LDFLAGS := -nostdlib -nostartfiles \
           -Wl,-m,elf_i386 -Wl,-T,linker.ld \
           -L$(LIBC_PATH)

LIBGCC := $(shell $(CC) -m32 -print-libgcc-file-name)

LIBS := -lc $(LIBGCC) -lm

ASFLAGS := -f elf32
DATE    := $(shell date)
ARCH    := $(shell gcc -dumpmachine)

CFLAGS := -g -m32 -ffreestanding -fno-stack-protector -fno-pic \
          -fno-leading-underscore -std=c99 -Wall -Wextra -U__TLS -fcommon \
          -I$(shell pwd)/../include -I$(INCLUDE_PATH) \
          -D__BUILD_DATE__="\"$(DATE)\"" -D__BUILD_ARCH__="\"$(ARCH)\"" -DNO_DIR
