#include "vga.h"
#include <arch.h>
#include <zjunix/utils.h>

const int VGA_CHAR_MAX_ROW = 32;
const int VGA_CHAR_MAX_COL = 128;
const int VGA_CHAR_ROW = 30;
const int VGA_CHAR_COL = 80;
int cursor_row;
int cursor_col;
int cursor_freq = 31;

void kernel_set_cursor() {
    *GPIO_CURSOR = ((cursor_freq & 0xff) << 16) + ((cursor_row & 0xff) << 8) + (cursor_col & 0xff);
}

void init_vga() {
    unsigned int w = 0x000fff00;
    cursor_row = cursor_col = 0;
    cursor_freq = 31;
    kernel_set_cursor();
}

void kernel_clear_screen(int scope) {
    unsigned int w = 0x000fff00;
    scope &= 31;
    cursor_col = 0;
    cursor_row = 0;
    kernel_set_cursor();
    kernel_memset_word(CHAR_VRAM, w, scope * VGA_CHAR_MAX_COL);
}

void kernel_scroll_screen() {
    unsigned int w = 0x000fff00;
    kernel_memcpy(CHAR_VRAM, (CHAR_VRAM + VGA_CHAR_MAX_COL), (VGA_CHAR_ROW - 2) * VGA_CHAR_MAX_COL * 4);
    kernel_memset_word((CHAR_VRAM + (VGA_CHAR_ROW - 2) * VGA_CHAR_MAX_COL), w, VGA_CHAR_MAX_COL);
}

void kernel_putchar_at(int ch, int fc, int bg, int row, int col) {
    unsigned int *p;
    row = row & 31;
    col = col & 127;
    p = CHAR_VRAM + row * VGA_CHAR_MAX_COL + col;
    *p = ((bg & 0xfff) << 20) + ((fc & 0xfff) << 8) + (ch & 0xff);
}

int kernel_putchar(int ch, int fc, int bg) {
    unsigned int w = 0x000fff00;
    if (ch == '\r')
        return ch;
    if (ch == '\n') {
        kernel_memset_word(CHAR_VRAM + cursor_row * VGA_CHAR_MAX_COL + cursor_col, w, VGA_CHAR_COL - cursor_col);
        cursor_col = 0;
        if (cursor_row == VGA_CHAR_ROW - 2) {
            kernel_scroll_screen();
        } else {
            cursor_row++;
#ifdef VGA_CALIBRATE
            kernel_putchar(' ', fc, bg);
#endif  // VGA_CALIBRATE
        }
    } else if (ch == '\t') {
        if (cursor_col >= VGA_CHAR_COL - 4) {
            kernel_putchar('\n', 0, 0);
        } else {
            kernel_memset_word(CHAR_VRAM + cursor_row * VGA_CHAR_MAX_COL + cursor_col, w, 4 - cursor_col & 3);
            cursor_col = (cursor_col + 4) & (-4);
        }
    } else {
        if (cursor_col == VGA_CHAR_COL) {
            kernel_putchar('\n', 0, 0);
        }
        kernel_putchar_at(ch, fc, bg, cursor_row, cursor_col);
        cursor_col++;
    }
    kernel_set_cursor();
    return ch;
}

int kernel_puts(const char *s, int fc, int bg) {
    int ret = 0;
    while (*s) {
        ret++;
        kernel_putchar(*s++, fc, bg);
    }
    return ret;
}

int kernel_putint(int x, int fc, int bg) {
    char buffer[12];
    char *ptr = buffer + 11;
    int neg = 0;
    buffer[11] = 0;
    if (x == 0) {
        kernel_putchar('0', fc, bg);
        return x;
    }
    if (x < 0) {
        neg = 1;
        x = -x;
    }
    while (x) {
        ptr--;
        *ptr = (x % 10) + '0';
        x /= 10;
    }
    if (neg) {
        ptr--;
        *ptr = '-';
    }
    kernel_puts(ptr, fc, bg);
    return x;
}

static const char *BIN_MAP = "01";
int kernel_putintb(unsigned int x, int fc, int bg) {
    char buffer[12];
    char *ptr = buffer + 11;
    int neg = 0;
    buffer[11] = 0;
    if (x == 0) {
        kernel_putchar('0', fc, bg);
        return x;
    }
    while (x) {
        ptr--;
        *ptr = BIN_MAP[x & 1];
        x >> = 1;
    }
    kernel_puts(ptr, fc, bg);
    return x;
}

static const char *OCT_MAP = "01234567";
int kernel_putinto(unsigned int x, int fc, int bg) {
    char buffer[12];
    char *ptr = buffer + 11;
    buffer[11] = 0;
    if (x == 0) {
        kernel_putchar('0', fc, bg);
        return x;
    }
    while (x) {
        ptr--;
        *ptr = OCT_MAP[x & 7];
        x >> = 3;
    }
    kernel_puts(ptr, fc, bg);
    return x;
}

static const char *HEX_MAP = "0123456789abcdef";
int kernel_putintx(unsigned int x, int fc, int bg) {
    char buffer[12];
    char *ptr = buffer + 11;
    buffer[11] = 0;
    if (x == 0) {
        kernel_putchar('0', fc, bg);
        return x;
    }
    while (x) {
        ptr--;
        *ptr = HEX_MAP[x & 15];
        x >> = 4;
    }
    kernel_puts(ptr, fc, bg);
    return x;
}

static const char *HEX_MAP_M = "0123456789ABCDEF";
int kernel_putintX(unsigned int x, int fc, int bg) {
    char buffer[12];
    char *ptr = buffer + 11;
    buffer[11] = 0;
    if (x == 0) {
        kernel_putchar('0', fc, bg);
        return x;
    }
    while (x) {
        ptr--;
        *ptr = HEX_MAP_M[x & 15];
        x >> = 4;
    }
    kernel_puts(ptr, fc, bg);
    return x;
}

//put float type
double kernel_putfloat(double x, int fc, int bg) {
    char buffer[12];
    char *ptr = buffer + 11;
    buffer[11] = 0;
    int count = 0;
    double y = x;
    while((int)y != 0)
    {
        y = y / 10;
        count++;
    }
    for(int i = 0; i < count && i < 10; i++)
    {
        y = y * 10;
        ptr--;
        *ptr = (int)y;
        y = y - (int)y;
    }
    ptr--;
    *ptr = '.'
    for(int i = 0; i < 10 - count; i++)
    {
        y = y * 10;
        ptr--;
        *ptr = (int)y;
        y = y -(int)y;
    }
    return x;
}

int kernel_vprintf(const char *format, va_list ap) {
    int cnt = 0;
    while (*format) {
        if (*format != '%') {
            kernel_putchar(*format++, 0xfff, 0);
        } else {
            format++;
            switch (*format) {
                case 'c': {
                    char valch = va_arg(ap, int);
                    kernel_putchar(valch, 0xfff, 0);
                    format++;
                    cnt++;
                    break;
                }
                case 'd': {
                    int valint = va_arg(ap, int);
                    kernel_putint(valint, 0xfff, 0);
                    format++;
                    cnt++;
                    break;
                }
                case 'b': {
                    int valint = va_arg(ap, int);
                    kernel_putintb(valint, 0xfff, 0);
                    format++;
                    cnt++;
                    break;
                }
                case 'o': {
                    int valint = va_arg(ap, int);
                    kernel_putinto(valint, 0xfff, 0);
                    format++;
                    cnt++;
                    break;
                }
                case 'b': {
                    int valint = va_arg(ap, int);
                    kernel_putintb(valint, 0xfff, 0);
                    format++;
                    cnt++;
                    break;
                }
                case 'f': {
                    int valint = va_arg(ap, int);
                    kernel_putfloat(valint, 0xfff, 0);
                    format++;
                    cnt++;
                    break;
                }
                case 'o': {
                    int valint = va_arg(ap, int);
                    kernel_putinto(valint, 0xfff, 0);
                    format++;
                    cnt++;
                    break;
                }
                case 'x': {
                    int valint = va_arg(ap, int);
                    kernel_putintx(valint, 0xfff, 0);
                    format++;
                    cnt++;
                    break;
                }
                case 'X': {
                    int valint = va_arg(ap, int);
                    kernel_putintX(valint, 0xfff, 0);
                    format++;;
                    cnt++;
                    break;
                }
                case 's': {
                    char *valstr = va_arg(ap, char *);
                    kernel_puts(valstr, 0xfff, 0);
                    format++;
                    cnt++;
                    break;
                }
                default: {
                    cnt = -1;
                    goto exit;
                }
            }
        }
    }
exit:
    return cnt;
}

int kernel_printf(const char *format, ...) {
    int cnt = 0;
    va_list ap;
    va_start(ap, format);
    cnt = kernel_vprintf(format, ap);
    va_end(ap);
    return cnt;
}
