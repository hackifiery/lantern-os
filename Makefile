MBR_BIN = /usr/share/syslinux/mbr.bin

.PHONY: all src img run

all: src run

src:
	$(MAKE) -C src all

ifneq ($(OS),Windows_NT)
img: src
	dd if=/dev/zero of=lanternos.img bs=1M count=10
	mkfs.vfat -F 16 lanternos.img
	syslinux --install lanternos.img
	mkdir -p mnt
	sudo mount -o loop lanternos.img mnt

	sudo cp /usr/share/syslinux/mboot.c32 mnt/
	sudo cp /usr/share/syslinux/libcom32.c32 mnt/
	sudo cp src/kern.bin mnt/kern.bin
	echo -e "default lantern\n  label lantern\n say Loading lanternOS...\n  kernel /mboot.c32\n  append /kern.bin" | sudo tee mnt/syslinux.cfg
	sudo umount mnt
run: src img
	qemu-system-i386 -drive format=raw,file=lanternos.img -m 4
else
img: src
run: src
	qemu-system-i386 -kernel src/kern.bin
endif


clean:
	$(MAKE) -C src clean
	rm -f lanternos.img
	rm -rf mnt
	rm -f src/*.bin
	rm -f *.bin