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
	echo -e "DEFAULT lantern\nLABEL lantern\n  SAY Loading lanternOS...\n  KERNEL /mboot.c32\n  APPEND /kern.bin" | sudo tee mnt/syslinux.cfg
	sudo umount mnt
else
img: src
endif

run: img src
	#qemu-system-i386 -drive format=raw,file=lanternos.img -m 16
	qemu-system-i386 -kernel src/kern.bin

clean:
	$(MAKE) -C src clean
	rm -f lanternos.img
	rm -rf mnt
	rm -f src/*.bin
	rm -f *.bin