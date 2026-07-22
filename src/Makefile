include settings.mk

OBJS := boot.o kernel.o io.o idt.o interrupts.o sys.o gdt.o gdt_flush.o ata.o ustar.o sh.o mem.o fd.o syscalls.o binary.o

all: kern.bin

kern.bin: kern.elf
	$(OBJCOPY) -O binary $< $@

kern.elf: $(OBJS) linker.ld
	$(LD) $(LDFLAGS) -o $@ $(OBJS) $(LIBS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

%.o: %.asm
	$(ASM) $(ASFLAGS) $< -o $@

clean:
	rm -f $(OBJS) kern.elf kern.bin hello