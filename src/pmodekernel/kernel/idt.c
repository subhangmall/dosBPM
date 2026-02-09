#include <stdint.h>
#include "memmgt.h"

#define NULL ((void*)0)

#pragma pack(push, 1)
struct InterruptDescriptor {
   uint16_t offset_1;        // offset bits 0..15
   uint16_t selector;        // a code segment selector in GDT or LDT
   uint8_t  zero;            // unused, set to 0
   uint8_t  type_attributes; // gate type, dpl, and p fields
   uint16_t offset_2;        // offset bits 16..31
};

struct IdtPtr {
    uint16_t limit;
    uint32_t base;
};

#pragma pack(pop)

struct InterruptDescriptor* idt;
struct IdtPtr idtr;

// initialize idt variable and set cpu's idt register to point to it
void initIDT() {
    idt = kcalloc(sizeof(struct InterruptDescriptor)*256);
    
    idtr.limit = (sizeof(struct InterruptDescriptor) * 256) - 1;
    idtr.base = (uint32_t)idt;
    
    asm volatile (
        "lidt %0"
        : 
        : "m" (idtr)
        : 
    );
}

void setIDTValue(uint8_t intNum, uint32_t address, uint8_t attr, uint16_t selector) {
    if (idt==NULL) initIDT();
    
    struct InterruptDescriptor new = {
        (uint16_t)(address & 0xFFFF),
        selector,
        0x00,
        attr,
        (uint16_t)((address >> 16) & 0xFFFF) 
        
    };

    idt[intNum] = new;
}