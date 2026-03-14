ASM      := nasm
CC       := i686-elf-gcc
LD       := i686-elf-ld
BIN      := kern.bin

ASFLAGS  := -f elf32
CFLAGS   := -m32 -ffreestanding -fno-stack-protector -nostdlib -fno-pic -std=c99
LDFLAGS  := -m elf_i386 -T linker.ld

OBJS := \
    boot.o \
    kernel.o \
    io.o \
    serial.o \
    idt.o \
    interrupts.o \
    string_utils.o \
    sys.o \
    gdt.o \
    gdt_flush.o

.PHONY: all clean

all: $(BIN)

$(BIN): $(OBJS)
	$(LD) $(LDFLAGS) $^ -o $@

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

%.o: %.asm
	$(ASM) $(ASFLAGS) $< -o $@

clean:
	rm -f *.o $(BIN)