#include <stdint.h>
#include "minemu/user_abi.h"

#define MSH_MAX_LINE 127u

static void put(const char *s, uint32_t n) {
    (void)ioctl(MINEMU_FD_UART_OUT, MINEMU_IOCTL_UART_WRITE, (void *)s, n);
}
static void putstr(const char *s) {
    uint32_t n = 0;
    while (s[n]) n++;
    put(s, n);
}
static char getch(void) {
    char c;
    while (ioctl(MINEMU_FD_UART_IN, MINEMU_IOCTL_UART_READ, (void *)&c, UINT32_C(1)) != 1) { }
    return c;
}
static int streq(const char *a, const char *b) {
    while (*a && *a == *b) { a++; b++; }
    return *a == *b;
}

static void run_line(char *line, uint32_t len) {
    char *words[MSH_MAX_LINE];
    uint32_t n = 0, i = 0;
    line[len] = '\0';
    while (i < len) {
        while (i < len && line[i] == ' ') line[i++] = '\0';
        if (i >= len) break;
        words[n++] = &line[i];
        while (i < len && line[i] != ' ') i++;
    }
    if (n == 0) return;

    if (streq(words[0], "echo")) {
        for (uint32_t w = 1; w < n; w++) {
            if (w > 1) put(" ", 1);
            putstr(words[w]);
        }
        put("\n", 1);
    } else {
        putstr("command not found: ");
        putstr(words[0]);
        put("\n", 1);
    }
}

void minemu_user_main(void) {
    char line[MSH_MAX_LINE + 1];
    uint32_t len = 0;

    putstr("msh> ");
    for (;;) {
        char c = getch();
        if (c == '\n') {
            if (len <= MSH_MAX_LINE) run_line(line, len);
            len = 0;
            putstr("msh> ");
        } else if (c == 0x08 || c == 0x7f) {
            if (len > 0) len--;
        } else {
            if (len < MSH_MAX_LINE) line[len] = c;
            len++;
        }
    }
}
