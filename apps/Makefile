include ../src/settings.mk
# App compiler flags
APP_CFLAGS := -m32 -ffreestanding -fno-stack-protector -fno-pic \
              -fno-leading-underscore -std=c99 -Wall -Wextra -U__TLS -fcommon \
              -I$(shell pwd)/../include -I$(INCLUDE_PATH)

# App linker flags – use our own linker script, no spec file
APP_LDFLAGS := -nostdlib -nostartfiles \
               -Wl,-m,elf_i386 -Wl,-T,app.ld -Wl,-e,main \
               -L$(LIBC_PATH) -lc -lgcc -lm

STUB_SRC := app_stub.c
STUB_OBJ := app_stub.o
APP_SRCS := $(wildcard *.c)
APP_OBJS := $(filter-out $(STUB_OBJ), $(APP_SRCS:.c=.o))
APP_ELFS := $(APP_OBJS:.o=.elf)

all: $(STUB_OBJ) $(APP_ELFS)

$(STUB_OBJ): $(STUB_SRC)
	$(CC) $(APP_CFLAGS) -c $< -o $@

%.o: %.c
	$(CC) $(APP_CFLAGS) -c $< -o $@

# do NOT include app.ld in $^ (it's passed via -T)
%.elf: %.o $(STUB_OBJ)
	$(LD) -o $@ $^ $(APP_LDFLAGS)

clean:
	rm -f $(STUB_OBJ) $(APP_OBJS) $(APP_ELFS)