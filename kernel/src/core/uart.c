#include <stdarg.h>
#include "uart.h"
#include "minemu/platform.h"
#include "minemu/irq.h"

#define RX_BUF_SIZE 256u
static volatile uint8_t  rx_buf[RX_BUF_SIZE];
static volatile uint32_t rx_head;   /* written only by the IRQ handler */
static volatile uint32_t rx_tail;   /* written only by uart_getc */

void uart_init(void) {
    MINEMU_UART0->control = MINEMU_UART_CONTROL_RX_IRQ_ENABLE;
}

void uart_putc(char c) {
    while (!(MINEMU_UART0->status & MINEMU_UART_STATUS_TX_READY)) { }
    MINEMU_UART0->tx_data = (uint32_t)(uint8_t)c;
}

void uart_puts(const char *s) {
    while (*s) uart_putc(*s++);
}

/* Runs in IRQ mode (IRQs already masked). Must drain ALL bytes,
   otherwise the interrupt fires again right after EOI. */
void uart_irq_handler(void) {
    while (MINEMU_UART0->status & MINEMU_UART_STATUS_RX_READY) {
        uint8_t b = (uint8_t)MINEMU_UART0->rx_data;
        uint32_t next = (rx_head + 1) % RX_BUF_SIZE;
        if (next != rx_tail) {          /* drop the byte if the buffer is full */
            rx_buf[rx_head] = b;
            rx_head = next;
        }
    }
}

/* Shared state: disable IRQs around the access */
int uart_getc(void) {
    int c = -1;
    minemu_irq_disable();
    if (rx_tail != rx_head) {
        c = rx_buf[rx_tail];
        rx_tail = (rx_tail + 1) % RX_BUF_SIZE;
    }
    minemu_irq_enable();
    return c;
}

static void print_uint(uint32_t v, uint32_t base) {
    char buf[11];
    int i = 0;
    do { buf[i++] = "0123456789abcdef"[v % base]; v /= base; } while (v);
    while (i--) uart_putc(buf[i]);
}

void kprintf(const char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    for (; *fmt; fmt++) {
        if (*fmt != '%') { uart_putc(*fmt); continue; }
        switch (*++fmt) {
        case 's': { const char *s = va_arg(ap, const char *); uart_puts(s ? s : "(null)"); break; }
        case 'c': uart_putc((char)va_arg(ap, int)); break;
        case 'd': { int32_t n = va_arg(ap, int32_t);
                    if (n < 0) { uart_putc('-'); print_uint(0u - (uint32_t)n, 10); }
                    else print_uint((uint32_t)n, 10);
                    break; }
        case 'u': print_uint(va_arg(ap, uint32_t), 10); break;
        case 'x': print_uint(va_arg(ap, uint32_t), 16); break;
        case '%': uart_putc('%'); break;
        case '\0': fmt--; break;
        default: uart_putc('%'); uart_putc(*fmt); break;
        }
    }
    va_end(ap);
}
