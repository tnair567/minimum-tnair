#include "minemu/irq.h"
#include "minemu/platform.h"
#include "uart.h"

typedef void (*irq_handler_t)(void);

static irq_handler_t handlers[4] = {
    [MINEMU_IRQ_UART0] = uart_irq_handler,
};

struct minemu_trap_frame *minemu_irq_dispatch(struct minemu_trap_frame *frame) {
    uint32_t source = (uint32_t)frame->exception_id;
    if (source < 4 && handlers[source])
        handlers[source]();
    if (source != MINEMU_IRQ_NONE)
        MINEMU_INTERRUPT->eoi = source;
    return frame;
}
