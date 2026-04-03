#ifndef LANTERN_IO_H
#define LANTERN_IO_H
#include "kstdint.h"

enum VGAColor {
    BLACK = 0, BLUE = 1, GREEN = 2, CYAN = 3,
    RED = 4, MAGENTA = 5, BROWN = 6, LIGHT_GRAY = 7,
    DARK_GRAY = 8, LIGHT_BLUE = 9, LIGHT_GREEN = 10,
    LIGHT_CYAN = 11, LIGHT_RED = 12, LIGHT_MAGENTA = 13,
    YELLOW = 14, WHITE = 15
};

static inline uint8_t vgaColor(enum VGAColor fg, enum VGAColor bg) {
    return (bg << 4) | (fg & 0x0F);
}

extern uint8_t COLOR;

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
extern uint8_t keymap[128], keymapShifted[128];
extern int shiftActive, capsLockActive;
extern int eofTrigger;
extern int blocking;

static inline void outb(uint16_t port, uint8_t val) {
    __asm__ volatile ("outb %0, %1" : : "a"(val), "Nd"(port));
}

static inline uint8_t inb(uint16_t port) {
    uint8_t ret;
    __asm__ volatile ("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

static inline void outw(uint16_t port, uint16_t data) {
    __asm__ volatile ("outw %w0, %w1" : : "a"(data), "Nd"(port));
}

static uint16_t inw(uint16_t port) {
    uint16_t result;
    __asm__ volatile ("inw %1, %0" : "=a"(result) : "dN"(port));
    return result;
}

void keyboardHandler(void);
void enableCursor(unsigned int cursor_start, unsigned int cursor_end);
void disableCursor(void);
char getInput(void);
void moveCursor(int x, int y);
void advanceCursor(void);
void writeChar(char c);
void writeStr(char s[]);
void clearScreen(void);
void fmtWrite(const char *fmt, ...);
void fmtGet(const char *fmt, ...);
void sfmtWrite(char* str, const char *fmt, ...);
void sfmtGet(const char *src, const char *fmt, ...);
int atoi(const char *s);

#endif // LANTERN_IO_H