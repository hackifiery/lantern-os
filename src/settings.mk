ASM      := nasm
BIN      := kern.bin

ifeq ($(OS),Windows_NT)
    CC      := i686-w64-mingw32-gcc
    LD      := i686-w64-mingw32-ld
    LDFLAGS := -m i386pe -T linker.ld
else
    CC      := i686-elf-gcc
    LD      := i686-elf-ld
    LDFLAGS := -m elf_i386 -T linker.ld
endif

#CC      := i686-w64-mingw32-gcc
#LD      := i686-w64-mingw32-ld
#LDFLAGS := -m i386pe -T linker.ld

OBJCOPY := objcopy
DATE     := $(shell date)
ARCH     := $(shell gcc -dumpmachine)
ASFLAGS  := -f elf32
CFLAGS   := -m32 -ffreestanding -fno-stack-protector -nostdlib -fno-pic -fno-leading-underscore -std=c99 \
            -I$(shell pwd)/../include -D__BUILD_DATE__="\"$(DATE)\"" -D__BUILD_ARCH__="\"$(ARCH)\""
#APPS     := calc
