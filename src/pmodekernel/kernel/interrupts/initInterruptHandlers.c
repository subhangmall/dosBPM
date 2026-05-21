#include <stdint.h>
#include "../logging.h"
#include "./idt.h"
#include "./intrStructs.h"

void divByZeroException(struct InterruptStackFrame* stack) {
    kprint("Division by zero error! Halting the system!");
    asm volatile (
        "hlt"
        :
        :
        :
    );
}

void pageFault(struct InterruptStackFrame* stack) {
    kprint("Page fault! Halting the system!");
    asm volatile ("hlt");
}

void initInterruptHandlers() {
    setIDTHandler(0, (uint32_t) &divByZeroException);
    setIDTHandler(14, (uint32_t) &pageFault);
}