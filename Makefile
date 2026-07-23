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
	$(MAKE) -C apps all

$(BOOT_BIN): $(BOOT_SRC)
	nasm -f bin $(BOOT_SRC) -o $(BOOT_BIN)

$(IMG): src $(BOOT_BIN) $(KERNEL_BIN)
	dd if=/dev/zero of=$(IMG) bs=1k count=4000
	dd if=$(BOOT_BIN) of=$(IMG) bs=512 seek=0 count=1 conv=notrunc
	dd if=$(KERNEL_BIN) of=$(IMG) bs=512 seek=1 count=100 conv=notrunc

	# Create test files
	echo "hello" > hello.txt
	mkdir -p folder
	echo "hi" > folder/hi.txt

	# Build apps and create TAR archive
	if [ "$(APPS)" = "1" ]; then \
		$(MAKE) -C apps all; \
		mv apps/hello.elf hello; \
		tar --format=ustar -cf archive.tar hello.txt hello; \
	else \
		tar --format=ustar -cf archive.tar hello.txt; \
	fi

	# Write TAR archive to disk image (starting at sector 101)
	dd if=archive.tar of=$(IMG) seek=101 bs=512 conv=notrunc

	# Clean up temporary files
	rm -rf hello.txt hello folder archive.tar

run: $(IMG)
	qemu-system-i386 -drive format=raw,file=$(IMG),index=0,if=ide -m 64 #-display curses

debug: $(IMG)
	qemu-system-i386 -drive format=raw,file=$(IMG),index=0,if=ide -m 16 -s -S #-display curses

clean:
	$(MAKE) -C src clean
	$(MAKE) -C apps clean
	rm -f $(IMG) $(BOOT_BIN)
	rm -rf hello.txt archive.tar folder