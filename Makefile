include src/settings.mk
BOOT_SRC = bl/bootloader.asm
BOOT_BIN = bl/bootloader.bin
KERNEL_BIN = src/kern.bin
IMG = lanternos.img

.PHONY: src img

all: src img

src:
	$(MAKE) -C src all
	$(MAKE) -C src/apps all

img: src
	nasm -f bin $(BOOT_SRC) -o $(BOOT_BIN)
	dd if=/dev/zero of=$(IMG) bs=1k count=4000
	dd if=$(BOOT_BIN) of=$(IMG) bs=512 seek=0 count=1 conv=notrunc
	dd if=$(KERNEL_BIN) of=$(IMG) bs=512 seek=1 count=100 conv=notrunc
	echo "hello" > hello.txt
	tar --format=ustar -cf archive.tar hello.txt calc hello
	dd if=archive.tar of=$(IMG) seek=101 bs=512 count=100 conv=notrunc
	rm -rf hello.txt archive.tar calc hello

run: img
	qemu-system-i386 \
		-drive format=raw,file=$(IMG),index=0,if=ide \
		-m 64 #-display curses

clean:
	$(MAKE) -C src clean
	$(MAKE) -C src/apps clean
	rm -f $(IMG) $(BOOT_BIN)
	rm -f **/*.o **/*.bin **/*.elf **/*.s calc hello hello.txt archive.tar
