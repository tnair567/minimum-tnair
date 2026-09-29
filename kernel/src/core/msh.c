#include "uart.h"

#define MSH_MAX_LINE 20u

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
            if (w > 1) uart_putc(' ');
            uart_puts(words[w]);
        }
        uart_putc('\n');
    } else {
        kprintf("command not found: %s\n", words[0]);
    }
}

void msh_run(void) {
    char line[MSH_MAX_LINE + 1];
    uint32_t len = 0;

    uart_puts("msh> ");
    for (;;) {
        int c = uart_getc();
        if (c < 0) continue;
        if (c == '\n') {
            if (len <= MSH_MAX_LINE) run_line(line, len);
            len = 0;
            uart_puts("msh> ");
        } else if (c == 0x08 || c == 0x7f) {
            if (len > 0) len--;
        } else {
            if (len < MSH_MAX_LINE) line[len] = (char)c;
            len++;
        }
    }
}
