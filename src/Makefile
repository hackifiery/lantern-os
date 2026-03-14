ASM = nasm
CC = i686-elf-gcc
LD = ld
ASFLAGS = -f elf32
CFLAGS = -m32 -c -ffreestanding -fno-stack-protector -nostdlib -fno-pic -std=c99
LDFLAGS = -m elf_i386 -T linker.ld
BIN = kern.bin

all: $(BIN) run

$(BIN): kernel.o kernelC.o io.o serial.o idt.o interrupts.o string_utils.o
	$(LD) $(LDFLAGS) kernel.o kernelC.o io.o serial.o idt.o interrupts.o string_utils.o -o $(BIN)

kernel.o: kernel.asm
	$(ASM) $(ASFLAGS) kernel.asm -o kernel.o

io.o: io.c
	$(CC) $(CFLAGS) io.c -o io.o

serial.o: serial.c
	$(CC) $(CFLAGS) serial.c -o serial.o

idt.o: idt.c
	$(CC) $(CFLAGS) idt.c -o idt.o

kernelC.o: kernel.c
	$(CC) $(CFLAGS) kernel.c -o kernelC.o

interrupts.o: interrupts.asm
	$(ASM) $(ASFLAGS) interrupts.asm -o interrupts.o

string_utils.o: string_utils.c
	$(CC) $(CFLAGS) string_utils.c -o string_utils.o

run: $(BIN)
	qemu-system-i386 -kernel $(BIN)

clean:
	rm -f *.o $(BIN)
