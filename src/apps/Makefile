include ../settings.mk

CFLAGS := -m32 -ffreestanding -nostdlib -nostdinc -nostartfiles -nodefaultlibs \
          -fno-stack-protector -fno-pic -std=c99 \
          -I../../include -Ttext 0x200000 -e main -Wl,--build-id=none

# cygwin patch
ifeq ($(OS),Windows_NT)
define COMPILE
	$(CC) $(CFLAGS) -S -o $(1).s $(2)
	sed -i 's/call\t___main/nop\n\tnop\n\tnop\n\tnop\n\tnop/' $(1).s
	$(CC) $(CFLAGS) -o $(1) $(1).s
endef
else
define COMPILE
	$(CC) $(CFLAGS) -o $(1) $(2)
endef
endif

all: calc.bin hello.bin

calc.elf: calc.c
	$(call COMPILE,calc.elf,calc.c)

calc.bin: calc.elf
	$(OBJCOPY) -O binary calc.elf calc.bin
	mv calc.bin ../../calc

hello.elf: hello.c
	$(call COMPILE,hello.elf,hello.c)

hello.bin: hello.elf
	$(OBJCOPY) -O binary hello.elf hello.bin
	mv hello.bin ../../hello

clean:
	rm -f *.elf *.bin *.s ../../calc ../../hello