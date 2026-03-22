include ../settings.mk
CC      := $(CC)
CFLAGS  := -m32 -ffreestanding -nostdlib -nostdinc -fno-stack-protector -fno-pic -std=c99 -I../../include -Ttext 0x200000 -e main -Wl,--build-id=none

all: calc.bin hello.bin

calc.elf: calc.c
	$(CC) $(CFLAGS) -o calc.elf calc.c

calc.bin: calc.elf
	$(OBJCOPY) -O binary calc.elf calc.bin
	mv calc.bin ../../calc

hello.elf: hello.c
	$(CC) $(CFLAGS) -o hello.elf hello.c

hello.bin: hello.elf
	$(OBJCOPY) -O binary hello.elf hello.bin
	mv hello.bin ../../hello

clean:
	rm -f *.elf *.bin calc.bin hello.bin