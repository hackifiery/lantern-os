include settings.mk
.PHONY: all

OBJS     := boot.o kernel.o io.o serial.o idt.o interrupts.o sys.o gdt.o gdt_flush.o ata.o ustar.o sh.o mem.o fd.o syscalls.o

all: $(BIN)

$(BIN): $(OBJS)
	$(LD) -m elf_i386 -T linker.ld $(OBJS) -L/usr/i686-elf/lib -lc -lg -lm -o kern.elf
	$(OBJCOPY) -O binary kern.elf $(BIN)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

%.o: %.asm
	$(ASM) $(ASFLAGS) $< -o $@

clean:
	rm -f *.o $(BIN) kern.elf