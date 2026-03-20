ASM      := nasm
BIN      := kern.bin

ifeq ($(OS),Windows_NT)
    CC      := gcc
    LD      := ld
	
    LDFLAGS := -m i386pe -T linker.ld
else
    CC      := i686-elf-gcc
    LD      := i686-elf-ld
    LDFLAGS := -m elf_i386 -T linker.ld
endif

HAS_CROSS = $(shell command -v i686-elf-gcc 2> /dev/null)

ifndef (HAS_CROSS)
	CC = gcc
	LD = ld
endif

OBJCOPY := objcopy
DATE     := $(shell date)
ARCH     := $(shell gcc -dumpmachine)
ASFLAGS  := -f elf32
CFLAGS   := -m32 -ffreestanding -fno-stack-protector -nostdlib -fno-pic -fno-leading-underscore -std=c99 -I../include -D__BUILD_DATE__="\"$(DATE)\"" -D__BUILD_ARCH__="\"$(ARCH)\""
OBJS     := boot.o kernel.o io.o serial.o idt.o interrupts.o string_utils.o sys.o gdt.o gdt_flush.o ata.o tar.o

.PHONY: all

all: $(BIN)

$(BIN): $(OBJS)
	$(LD) $(LDFLAGS) $(OBJS) -o kern.elf
	$(OBJCOPY) -O binary kern.elf $(BIN)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

%.o: %.asm
	$(ASM) $(ASFLAGS) $< -o $@

clean:
	rm -f *.o $(BIN)
