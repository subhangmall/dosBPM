#include <stdint.h>
#include "./kmemmgt.h"
#include "./logging.h"

#define NULL ((void*)0)

#pragma pack(push, 1)
struct InterruptDescriptor {
   uint16_t offset1;        // offset bits 0..15
   uint16_t selector;        // a code segment selector in GDT or LDT
   uint8_t  zero;            // unused, set to 0
   uint8_t  type_attributes; // gate type, dpl, and p fields
   uint16_t offset2;        // offset bits 16..31
};

struct IdtPtr {
    uint16_t limit;
    uint32_t base;
};

#pragma pack(pop)

struct InterruptDescriptor idt[256] __attribute__((aligned(16)));
struct IdtPtr idtr;

extern void int0(void);
extern void int1(void);
extern void int2(void);
extern void int3(void);
extern void int4(void);
extern void int5(void);
extern void int6(void);
extern void int7(void);
extern void int8(void);
extern void int9(void);
extern void intA(void);
extern void intB(void);
extern void intC(void);
extern void intD(void);
extern void intE(void); // SKIP F
extern void int10(void);
extern void int11(void);
extern void int12(void);
extern void int13(void);
extern void int14(void);
extern void int15(void);

// initialize idt variable and set cpu's idt register to point to it
void initIDTStructures() {   
    struct InterruptDescriptor zeroIDT = {
            0x0000,
            0x0000,
            0x00,
            0x00,
            0x0000
    };
    for (int i = 0; i < 256; i++) {
        idt[i] = zeroIDT;
    }

    idtr.limit = (sizeof(struct InterruptDescriptor) * 256) - 1;
    idtr.base = (uint32_t) &idt;
    
    
}

void setIDTValue(uint8_t intNum, uint32_t address, uint8_t attr, uint16_t selector) {
    if (idt==NULL) initIDTStructures();
    
    struct InterruptDescriptor new = {
        (uint16_t)(address & 0xFFFF),
        selector,
        0x00,
        attr,
        (uint16_t)((address >> 16) & 0xFFFF) 
        
    };

    idt[intNum] = new;
}

void commonHandler(uint32_t* stack) {
    kprint("Interrupt number: ");
    kprint_hex(stack[12]);
    kprint("\n");
    // error code is stack[13]
    return;
}

void initIDT() {
    setIDTValue(0x00, (uint32_t) &int0, 0x8E, 0x08);
    setIDTValue(0x01, (uint32_t) &int1, 0x8E, 0x08);
    setIDTValue(0x02, (uint32_t) &int2, 0x8E, 0x08);
    setIDTValue(0x03, (uint32_t) &int3, 0x8E, 0x08);
    setIDTValue(0x04, (uint32_t) &int4, 0x8E, 0x08);
    setIDTValue(0x05, (uint32_t) &int5, 0x8E, 0x08);
    setIDTValue(0x06, (uint32_t) &int6, 0x8E, 0x08);
    setIDTValue(0x07, (uint32_t) &int7, 0x8E, 0x08);
    setIDTValue(0x08, (uint32_t) &int8, 0x8E, 0x08);
    setIDTValue(0x09, (uint32_t) &int9, 0x8E, 0x08);
    setIDTValue(0x0A, (uint32_t) &intA, 0x8E, 0x08);
    setIDTValue(0x0B, (uint32_t) &intB, 0x8E, 0x08);
    setIDTValue(0x0C, (uint32_t) &intC, 0x8E, 0x08);
    setIDTValue(0x0D, (uint32_t) &intD, 0x8E, 0x08);
    setIDTValue(0x0E, (uint32_t) &intE, 0x8E, 0x08);
    setIDTValue(0x10, (uint32_t) &int10, 0x8E, 0x08);
    setIDTValue(0x11, (uint32_t) &int11, 0x8E, 0x08);
    setIDTValue(0x12, (uint32_t) &int12, 0x8E, 0x08);
    setIDTValue(0x13, (uint32_t) &int13, 0x8E, 0x08);
    setIDTValue(0x14, (uint32_t) &int14, 0x8E, 0x08);
    setIDTValue(0x15, (uint32_t) &int15, 0x8E, 0x08);

    asm volatile (
        "lidt %0"
        : 
        : "m" (idtr)
        : 
    );
}