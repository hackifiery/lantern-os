#include "io.h"
#include "serial.h"

enum VGAColor {
    BLACK = 0, BLUE = 1, GREEN = 2, CYAN = 3,
    RED = 4, MAGENTA = 5, BROWN = 6, LIGHT_GRAY = 7,
    DARK_GRAY = 8, LIGHT_BLUE = 9, LIGHT_GREEN = 10,
    LIGHT_CYAN = 11, LIGHT_RED = 12, LIGHT_MAGENTA = 13,
    YELLOW = 14, WHITE = 15
};

static inline unsigned char vgaColor(enum VGAColor fg, enum VGAColor bg) {
    return (bg << 4) | (fg & 0x0F);
}

#define COLOR vgaColor(GREEN, BLACK)

#define SERIAL 1
#define BUFFER_SIZE 256

unsigned int cursorX = 0, cursorY = 0;

volatile char keyBuffer[BUFFER_SIZE];
volatile int head = 0; // Where the interrupt writes
volatile int tail = 0; // Where the program reads
int shiftActive = 0;
int capsLockActive = 0;

unsigned char keymap[128] = {
    0,  27, '1', '2', '3', '4', '5', '6', '7', '8',	'9', '0', '-', '=', '\b',
  '\t', 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',
    0, 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`',   0,
 '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/',   0, '*',   0, ' '
};

unsigned char keymapShifted[128] = {
    0,  27, '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+', '\b',
  '\t', 'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', '\n',
    0,  'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ':', '\"', '~',   0,
 '|', 'Z', 'X', 'C', 'V', 'B', 'N', 'M', '<', '>', '?',   0, '*',   0, ' '
};

void keyboardHandler(void) {
    unsigned char scancode = inb(0x60);

    // Check for "Break" codes (Key Released)
    if (scancode & 0x80) {
        unsigned char released = scancode & 0x7F;
        if (released == 0x2A || released == 0x36) {
            shiftActive = 0;
        }
    } 
    // Check for "Make" codes (Key Pressed)
    else {
        if (scancode == 0x2A || scancode == 0x36) {
            shiftActive = 1;
            goto end; // Don't put Shift into the text buffer
        }
        if (scancode == 0x3A) {
            capsLockActive = !capsLockActive; // Toggle Caps
            goto end;
        }

        char c;
        // TODO: capslock only affects letters
        if (shiftActive ^ capsLockActive) {
            c = keymapShifted[scancode];
        }
        else {
            c = keymap[scancode];
        }

        if (c > 0) {
            int next = (head + 1) % BUFFER_SIZE;
            if (next != tail) {
                keyBuffer[head] = c;
                head = next;
            }
        }
    }

end:
    outb(0x20, 0x20);
}
char getInput(void) {
    if (head == tail) return 0; // Buffer empty

    char c = keyBuffer[tail];
    tail = (tail + 1) % BUFFER_SIZE;
    return c;
}
void enableCursor(unsigned int cursor_start, unsigned int cursor_end) {
	outb(0x3D4, 0x0A);
	outb(0x3D5, (inb(0x3D5) & 0xC0) | cursor_start);

	outb(0x3D4, 0x0B);
	outb(0x3D5, (inb(0x3D5) & 0xE0) | cursor_end);
}

void moveCursor(int x, int y) {
    unsigned short pos = y * VGA_W + x;

    outb(0x3D4, 0x0E); // Register 0x0E: Cursor Location High
    outb(0x3D5, (unsigned char)((pos >> 8) & 0xFF));

    outb(0x3D4, 0x0F); // Register 0x0F: Cursor Location Low
    outb(0x3D5, (unsigned char)(pos & 0xFF));
}

void advanceCursor(void) {
    if (cursorX >= VGA_W) {
        cursorY++;
        cursorX = 0;
    }
    else cursorX++;
    moveCursor(cursorX, cursorY);
}

void writeChar(char c) {
    if (c == '\n') {
        cursorX = 0;
        cursorY++;
    }
    else if (c == '\b') {
        if (cursorX > 0) {
            cursorX--;
        }
        else if (cursorY > 0) {
            cursorY--;
            cursorX = VGA_W - 1;
        }
        // Erase the character at the new position
        int offset = VGA_OFF;
        VGA_START[offset] = ' ';
        VGA_START[offset + 1] = COLOR;
    }
    else {
        int offset = VGA_OFF;
        
        VGA_START[offset] = c;
        VGA_START[offset + 1] = COLOR;
        
        cursorX++;
    }

    if (cursorX >= VGA_W) {
        cursorX = 0;
        cursorY++;
    }
    if (SERIAL) writeSerial(c);
    moveCursor(cursorX, cursorY);
}

void writeStr(char s[]) {
    for (int i = 0; s[i] != '\0'; i++)
        writeChar(s[i]);
}

void clearScreen(void) {
    char *video_memory = VGA_START;
    
    // 80 columns * 25 VGA_H = 2,000 characters
    for (int i = 0; i < VGA_W * VGA_H; i++) {
        // Multiplied by 2 because each character has 2 bytes (char + attr)
        video_memory[i * 2] = ' ';
        video_memory[i * 2 + 1] = COLOR;
    }
    cursorX = 0, cursorY = 0;
    moveCursor(0, 0);
}



static int atoi(const char *s) {
    int res = 0;
    int sign = 1;

    // Handle negative numbers
    if (*s == '-') {
        sign = -1;
        s++;
    }

    while (*s >= '0' && *s <= '9') {
        res = res * 10 + (*s - '0');
        s++;
    }

    return res * sign;
}

static int intToStr(char *dest, int n) {
    int i = 0;
    if (n == 0) {
        dest[i++] = '0';
        return i;
    }
    if (n < 0) {
        dest[i++] = '-';
        n = -n;
    }

    char tmp[11];
    int j = 0;
    while (n > 0) {
        tmp[j++] = (n % 10) + '0';
        n /= 10;
    }
    // Reverse tmp into dest
    while (--j >= 0) {
        dest[i++] = tmp[j];
    }
    return i;
}

static void vsfmtWrite(char* str, const char *fmt, va_list args) {
    int ptr = 0;
    for (int i = 0; fmt[i] != '\0'; i++) {
        if (fmt[i] == '%') {
            i++;
            switch (fmt[i]) {
                case 's': {
                    char *s = va_arg(args, char*);
                    while (*s) str[ptr++] = *s++;
                    break;
                }
                case 'd': {
                    ptr += intToStr(&str[ptr], va_arg(args, int));
                    break;
                }
                case 'c': {
                    str[ptr++] = (char)va_arg(args, int);
                    break;
                }
                case '%': {
                    str[ptr++] = '%';
                    break;
                }
            }
        } else {
            str[ptr++] = fmt[i];
        }
    }
    str[ptr] = '\0';
}

void sfmtWrite(char* str, const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    vsfmtWrite(str, fmt, args);
    va_end(args);
}

void fmtWrite(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    char buf[256];      // Buffer large enough for a line
    vsfmtWrite(buf, fmt, args);
    writeStr(buf);
    va_end(args);
}

static void vsfmtGet(const char *src, const char *fmt, va_list args) {
    int s_ptr = 0;
    for (int i = 0; fmt[i] != '\0'; i++) {
        if (fmt[i] == '%') {
            i++;
            switch (fmt[i]) {
                case 'd': {
                    int *dest = va_arg(args, int*);
                    *dest = atoi(&src[s_ptr]);
                    if (src[s_ptr] == '-') s_ptr++;
                    while (src[s_ptr] >= '0' && src[s_ptr] <= '9') s_ptr++;
                    break;
                }
                case 's': {
                    char *dest = va_arg(args, char*);
                    while (src[s_ptr] != '\0' && src[s_ptr] != '\n') {
                        *dest++ = src[s_ptr++];
                    }
                    *dest = '\0';
                    break;
                }
                case 'c': {
                    char *dest = va_arg(args, char*);
                    *dest = src[s_ptr++];
                    break;
                }
            }
        } else {
            if (src[s_ptr] == fmt[i]) {
                s_ptr++;
            }
        }
    }
}

void sfmtGet(const char *src, const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    vsfmtGet(src, fmt, args);
    va_end(args);
}

void fmtGet(const char *fmt, ...) {
    char input_buf[256];
    
    // First, read the raw line from the keyboard
    int i = 0;
    while (1) {
        char c = 0;
        while (!(c = getInput())); // Wait for key
        
        if (c == '\n') {
            input_buf[i] = '\0';
            writeChar('\n');
            break;
        } else if (c == '\b') {
            if (i > 0) { i--; writeChar('\b'); }
        } else if (i < 255) {
            input_buf[i++] = c;
            writeChar(c);
        }
    }

    // Now parse it
    va_list args;
    va_start(args, fmt);
    vsfmtGet(input_buf, fmt, args);
    va_end(args);
}