#include "io.h"
#include "kstdint.h"
#include "serial.h"

#define SERIAL 1
#define BUFFER_SIZE 256

uint8_t COLOR = vgaColor(GREEN, BLACK);

unsigned int cursorX = 0, cursorY = 0;

volatile char keyBuffer[BUFFER_SIZE];
volatile int head = 0; // Where the interrupt writes
volatile int tail = 0; // Where the program reads
int shiftActive = 0;
int capsLockActive = 0;
int controlActive = 0;  // 0 = released, 1 = pressed
int eofTrigger = 0;
int blocking = 1;
char arrow = '\0';

uint8_t keymap[128] = {
    0,  27, '1', '2', '3', '4', '5', '6', '7', '8',	'9', '0', '-', '=', '\b',
  '\t', 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',
    0, 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`',   0,
 '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/',   0, '*',   0, ' '
};

uint8_t keymapShifted[128] = {
    0,  27, '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+', '\b',
  '\t', 'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', '\n',
    0,  'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ':', '\"', '~',   0,
 '|', 'Z', 'X', 'C', 'V', 'B', 'N', 'M', '<', '>', '?',   0, '*',   0, ' '
};

static void scroll(void) {
    // move every line up by one
    for (int y = 0; y < VGA_H - 1; y++) {
        for (int x = 0; x < VGA_W; x++) {
            int dst = (y * VGA_W + x) * 2;
            int src = ((y + 1) * VGA_W + x) * 2;
            VGA_START[dst]     = VGA_START[src];
            VGA_START[dst + 1] = VGA_START[src + 1];
        }
    }
    // clear the last line
    for (int x = 0; x < VGA_W; x++) {
        int offset = ((VGA_H - 1) * VGA_W + x) * 2;
        VGA_START[offset]     = ' ';
        VGA_START[offset + 1] = COLOR;
    }
    cursorY = VGA_H - 1;
}

void keyboardHandler(void) {
    uint8_t scancode = inb(0x60);
    //fmtWrite("scancode: %02x or %2d\n", scancode, scancode);

    // Check for "Break" codes (Key Released)
    if (scancode & 0x80) {
        uint8_t released = scancode & 0x7F;
        if (released == 0x2A || released == 0x36) {
            shiftActive = 0;
        }
        if (released == 0x1D) controlActive = 0;
        if (released == 0x48 || released == 0x50 || released == 0x4B || released == 0x4D) { // arrows
            arrow = '\0';
        }
    } 
    // Check for "Make" codes (Key Pressed)
    else {
        switch (scancode) {
            case 0x2A: case 0x36: {
                shiftActive = 1;
                goto end; // Don't put Shift into the text buffer
            }
            case 0x3A: {
                capsLockActive = !capsLockActive; // Toggle Caps
                goto end;
            }
            case 0x1D: { // ctrl press
                controlActive = 1;
                goto end;
            }
            case 0x9D: { // ctrl release
                controlActive = 0;
                goto end;
            }
            case 0x48: { // up arrow
                arrow = 'u';
                goto end;
            }
            case 0x50: { // down arrow
                arrow = 'd';
                goto end;
            }
            case 0x4B: { // l arrow
                arrow = 'l';
                goto end;
            }
            case 0x4D: { // r arrow
                arrow = 'r';
                goto end;
            }
        }
        char c;
        if (controlActive) {
            char baseChar = keymap[scancode];
            
            // Check if it's a lowercase or uppercase letter
            if (baseChar >= 'a' && baseChar <= 'z') {
                c = baseChar - 'a' + 1; // 'a' (97) - 97 + 1 = 1
            } else if (baseChar >= 'A' && baseChar <= 'Z') {
                c = baseChar - 'A' + 1; // 'A' (65) - 65 + 1 = 1
            } else {
                c = baseChar; // Not a letter
            }

            switch (c) {
                case 12: // Ctrl + L
                    clearScreen();
                    break;
                case 4: // Ctrl + D
                    eofTrigger = 1;
                    break;
            }
            goto end; 
        }
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
    // fmtWrite("arrow: %c\n", arrow);
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

void disableCursor(void) {
	outb(0x3D4, 0x0A);
	outb(0x3D5, 0x20);
}

void moveCursor(int x, int y) {
    uint16_t pos = y * VGA_W + x;

    outb(0x3D4, 0x0E); // Register 0x0E: Cursor Location High
    outb(0x3D5, (uint8_t)((pos >> 8) & 0xFF));

    outb(0x3D4, 0x0F); // Register 0x0F: Cursor Location Low
    outb(0x3D5, (uint8_t)(pos & 0xFF));
}

void advanceCursor(void) {
    if (cursorX >= VGA_W) {
        cursorY++;
        cursorX = 0;
    }
    else if (cursorY >= VGA_H) scroll();
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
    if (cursorY >= VGA_H) scroll();
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




int atoi(const char *s) {
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

static int hexToStr(char *dest, unsigned int n, int width) {
    char *hexChars = "0123456789ABCDEF";
    char tmp[9]; 
    int i = 0, j = 0;

    if (n == 0) {
        tmp[j++] = '0';
    } else {
        while (n > 0) {
            tmp[j++] = hexChars[n % 16];
            n /= 16;
        }
    }

    // only pads if j (curr length) is less than width
    while (j < width && j < 8) {
        tmp[j++] = '0';
    }

    int count = j;
    while (--j >= 0) {
        dest[i++] = tmp[j];
    }
    return count;
}

static void vsfmtWrite(char* str, const char *fmt, va_list args) {
    int ptr = 0;
    for (int i = 0; fmt[i] != '\0'; i++) {
        if (fmt[i] == '%') {
            i++;

            char padChar = ' ';
            int width = 0;

            // check for zero padding
            if (fmt[i] == '0') {
                padChar = '0';
                i++;
            }

            // read width digits
            while (fmt[i] >= '0' && fmt[i] <= '9') {
                width = width * 10 + (fmt[i] - '0');
                i++;
            }

            switch (fmt[i]) {
                case 's': {
                    char *s = va_arg(args, char*);
                    while (*s) str[ptr++] = *s++;
                    break;
                }
                case 'd': {
                    char tmp[32];
                    int len = intToStr(tmp, va_arg(args, int));
                    while (len < width) {
                        str[ptr++] = padChar;
                        width--;
                    }
                    for (int j = 0; j < len; j++) str[ptr++] = tmp[j];
                    break;
                }
                case 'c': {
                    str[ptr++] = (char)va_arg(args, int);
                    break;
                }
                case 'x': {
                    ptr += hexToStr(&str[ptr], va_arg(args, unsigned int), width);
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
                    while (src[s_ptr] != '\0' /*&& src[s_ptr] != '\n'*/) {
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
    
    int i = 0;
    while (1) {
        char c = 0;
        while (!(c = getInput())) {
            if (eofTrigger) { writeChar('\n'); goto done; }  // break out of both loops
        }
        
        if (c == '\n' && blocking) {
            writeChar('\n');
            //input_buf[i++] = '\n';
            break;
        } else if (c == '\n' && !blocking) {
            input_buf[i++] = '\n';
            writeChar('\n');
        } else if (c == '\b') {
            if (i > 0) { i--; writeChar('\b'); }
        } else if (i < 255) {
            input_buf[i++] = c;
            writeChar(c);
        }
    }

done:
    eofTrigger = 0;
    input_buf[i] = '\0';
    //writeChar('\n');

    va_list args;
    va_start(args, fmt);
    vsfmtGet(input_buf, fmt, args);
    va_end(args);
}