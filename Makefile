BOOT_SRC = bl/bootloader.asm
BOOT_BIN = bl/bootloader.bin
KERNEL_BIN = src/kern.bin
IMG = lanternos.img
DISK_IMG = disk.img

.PHONY: src img

all: src img

src:
	$(MAKE) -C src all

img: src
	nasm -f bin $(BOOT_SRC) -o $(BOOT_BIN)
	dd if=/dev/zero of=$(IMG) bs=1k count=1440
	dd if=$(BOOT_BIN) of=$(IMG) conv=notrunc
	dd if=$(KERNEL_BIN) of=$(IMG) seek=1 conv=notrunc

$(DISK_IMG):
	dd if=/dev/zero of=$(DISK_IMG) bs=1M count=10
	mkdir -p etc
	echo "hello" > etc/hello.txt
	tar --format=ustar -cf archive.tar etc/hello.txt
	dd if=archive.tar of=$(DISK_IMG) conv=notrunc
	rm -rf etc archive.tar

run: img src $(DISK_IMG)
	qemu-system-i386 \
		-drive format=raw,file=$(IMG),if=floppy \
		-drive format=raw,file=$(DISK_IMG),index=0,if=ide \
		-m 64 -display curses

clean:
	$(MAKE) -C src clean
	rm -f $(IMG) $(DISK_IMG) $(BOOT_BIN)
	rm -f **/*.o **/*.bin **/*.elf