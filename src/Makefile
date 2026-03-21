include settings.mk
.PHONY: all

OBJS     := boot.o kernel.o io.o serial.o idt.o interrupts.o string_utils.o sys.o gdt.o gdt_flush.o ata.o tar.o

all: $(BIN)

$(BIN): $(OBJS)
	$(LD) $(LDFLAGS) $(OBJS) -o kern.elf
	$(OBJCOPY) -O binary kern.elf $(BIN)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

%.o: %.asm
	$(ASM) $(ASFLAGS) $< -o $@

clean:
	rm -f *.o $(BIN)
