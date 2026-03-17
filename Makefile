BOOT_SRC = bl/bootloader.asm
BOOT_BIN = bl/bootloader.bin
KERNEL_BIN = src/kern.bin
IMG = lanternos.img

.PHONY: all src img run clean

all: src img run
kern: src kernel

src:
	$(MAKE) -C src all

img: src
	nasm -f bin $(BOOT_SRC) -o $(BOOT_BIN)
	
	dd if=/dev/zero of=$(IMG) bs=1k count=1440
	dd if=$(BOOT_BIN) of=$(IMG) conv=notrunc
	dd if=$(KERNEL_BIN) of=$(IMG) seek=1 conv=notrunc

run: img
	qemu-system-i386 -drive format=raw,file=$(IMG),index=0,if=floppy -m 16 -display curses -vga std # -nographic -serial mon:stdio
kernel: src
	qemu-system-i386 -kernel src/kern.bin -m 2

clean:
	$(MAKE) -C src clean
	rm -f $(IMG)
	rm -f $(BOOT_BIN)