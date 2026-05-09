ASM      := nasm
BIN      := kern.bin

# TODO: windows support
# ifeq ($(OS),Windows_NT)
#     CC      := i686-w64-mingw32-gcc
#     LD      := i686-w64-mingw32-ld
#     LDFLAGS := -m i386pe -T linker.ld
# else
#     CC      := i686-elf-gcc
#     LD      := i686-elf-ld
#     LDFLAGS := -m elf_i386 -T linker.ld
# endif

LDFLAGS := -m elf_i386 -T linker.ld -L/usr/i686-elf/lib -lc -lg -lm
CC := i686-elf-gcc
LD := i686-elf-ld

#CC      := i686-w64-mingw32-gcc
#LD      := i686-w64-mingw32-ld
#LDFLAGS := -m i386pe -T linker.ld

OBJCOPY := objcopy
DATE     := $(shell date)
ARCH     := $(shell gcc -dumpmachine)
ASFLAGS  := -f elf32
CFLAGS   := -m32 -ffreestanding -fno-stack-protector -fno-pic -fno-leading-underscore -std=c99 \
            -I$(shell pwd)/../include -I/usr/i686-elf/include -D__BUILD_DATE__="\"$(DATE)\"" -D__BUILD_ARCH__="\"$(ARCH)\"" -DNO_DIR
