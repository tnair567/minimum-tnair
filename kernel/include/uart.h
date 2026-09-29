#ifndef UART_H
#define UART_H
#include <stdint.h>

void uart_init(void);          /* enable RX interrupts on UART0 */
void uart_putc(char c);        /* blocking: waits for TX ready */
void uart_puts(const char *s);
int  uart_getc(void);          /* non-blocking: -1 if nothing buffered */
void uart_irq_handler(void);   /* called by the IRQ dispatcher */
void kprintf(const char *fmt, ...);  /* supports %s %c %d %u %x %% */
void msh_run(void);
#endif
