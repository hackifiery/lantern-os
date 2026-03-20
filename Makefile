BOOT_SRC = bl/bootloader.asm
BOOT_BIN = bl/bootloader.bin
KERNEL_BIN = src/kern.bin
IMG = lanternos.img
DISK_IMG = disk.img

.PHONY: src img

all: src img

src:
	$(MAKE) -C src all

# This creates your OS floppy
img: src
	nasm -f bin $(BOOT_SRC) -o $(BOOT_BIN)
	dd if=/dev/zero of=$(IMG) bs=1k count=1440
	dd if=$(BOOT_BIN) of=$(IMG) conv=notrunc
	dd if=$(KERNEL_BIN) of=$(IMG) seek=1 conv=notrunc

# This creates a blank 10MB hard drive for testing your ATA driver
$(DISK_IMG):
	dd if=/dev/zero of=$(DISK_IMG) bs=1M count=10

# Added $(DISK_IMG) as a dependency for run
run: img src $(DISK_IMG)
	qemu-system-i386 \
		-drive format=raw,file=$(IMG),index=0,if=floppy \
		-drive format=raw,file=$(DISK_IMG),index=0,if=ide \
		-m 16 -display curses

clean:
	$(MAKE) -C src clean
	rm -f $(IMG) $(DISK_IMG) $(BOOT_BIN)
	rm -f **/*.o **/*.bin **/*.elf