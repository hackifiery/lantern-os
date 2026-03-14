#define VGA_W 80
#define VGA_H 25
// Pointer to the start of VGA video memory
#define VGA_START ((char *) 0xB8000)
#define VGA_OFF ((cursorY * VGA_W + cursorX) * 2)

typedef __builtin_va_list va_list;
#define va_start(v,l) __builtin_va_start(v,l)
#define va_arg(v,l)   __builtin_va_arg(v,l)
#define va_end(v)     __builtin_va_end(v)

extern unsigned int cursorX, cursorY;
extern unsigned char keymap[128];

static inline void outb(unsigned short port, unsigned char val) {
    __asm__ volatile ( "outb %0, %1" : : "a"(val), "Nd"(port) );
}

static inline unsigned char inb(unsigned short port) {
    unsigned char ret;
    __asm__ volatile ( "inb %1, %0" : "=a"(ret) : "Nd"(port) );
    return ret;
}

void keyboardHandler();
void enableCursor(unsigned int cursor_start, unsigned int cursor_end);
char getInput(void);
void moveCursor(unsigned int x, unsigned int y);
void advanceCursor(void);
void writeChar(char c);
void writeStr(char s[]);
void clearScreen(void);
void fmtWrite(const char *fmt, ...);
void fmtGet(const char *fmt, ...);
void sfmtWrite(char* str, const char *fmt, ...);
void sfmtGet(const char *src, const char *fmt, ...);