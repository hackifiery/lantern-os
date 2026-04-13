include src/settings.mk

BOOT_SRC    = bl/bootloader.asm
BOOT_BIN    = bl/bootloader.bin
KERNEL_BIN  = src/kern.bin
IMG         = lanternos.img
APPS ?= 1

.PHONY: all src apps clean run bare

all: APPS = 1
all: $(IMG)

bare: APPS = 0
bare: $(IMG)

src:
	$(MAKE) -C src all

apps:
	$(MAKE) -C src/apps all

$(BOOT_BIN): $(BOOT_SRC)
	nasm -f bin $(BOOT_SRC) -o $(BOOT_BIN)

$(IMG): src $(BOOT_BIN) $(KERNEL_BIN)
	dd if=/dev/zero of=$(IMG) bs=1k count=4000
	dd if=$(BOOT_BIN) of=$(IMG) bs=512 seek=0 count=1 conv=notrunc
	dd if=$(KERNEL_BIN) of=$(IMG) bs=512 seek=1 count=100 conv=notrunc
	
	echo "hello" > hello.txt
	mkdir folder
	echo "hi" > folder/hi.txt
	if [ "$(APPS)" = "1" ]; then \
		$(MAKE) -C src/apps all; \
		tar --format=ustar -cf archive.tar hello.txt calc hello folder; \
	else \
		tar --format=ustar -cf archive.tar hello.txt folder; \
	fi
	
	dd if=archive.tar of=$(IMG) seek=101 bs=512 count=128 conv=notrunc
	rm -rf hello.txt folder #archive.tar

run: $(IMG)
	qemu-system-i386 -drive format=raw,file=$(IMG),index=0,if=ide -m 512

clean:
	$(MAKE) -C src clean
	$(MAKE) -C src/apps clean
	rm -f $(IMG) $(BOOT_BIN)
	rm -rf hello.txt archive.tar folder