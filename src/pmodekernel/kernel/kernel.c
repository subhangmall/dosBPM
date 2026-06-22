// TODO: FIX WEIRD MEMORY

#include <stdint.h>
#include <stdbool.h>
#include <kernel/memory/memSetup.h>
// #include "./memory/kmemmgt.h"
// #include "./idt.h"
#include <kernel/logging.h>
#include <kernel/interrupts/idt.h>
#include <kernel/interrupts/pic.h>
#include <kernel/memory/vmm.h>
#include <kernel/memory/kalloc.h>
#include <kernel/interrupts/initInterruptHandlers.h>
#include <kernel/time/time.h>
#include <kernel/memory/pmm.h>
// #include "./gdt.asm"

// extern void int0(void);
// #include "./memory/kmemmgt.h"
// #include "./logging.h"
// #include "./idt.h"
// #include "./pic.h"
// #include "./basicInterruptHandlers.h"
extern void gdtFlush();

__attribute__((section(".boot"))) void kentry(uint32_t dummyCS, uint32_t e820LenAddr, uint32_t e820StartAddress);
void continueInitialization();

__attribute__((section(".boot"))) void kentry(uint32_t dummyCS, uint32_t e820LenAddr, uint32_t e820StartAddress) { // dummy CS because CS Is also pushed onto the stack cause it is called from a far jump
    initMemory(&continueInitialization, e820LenAddr, e820StartAddress);
}

void continueInitialization() {  
    // kclear();

    setupInterruptStructures();
    initPIC(0x20, 0x28);
    // disablePIC();
    enableInterrupts();
    // asm volatile (
    //     "int $0x40"
    //     :
    //     :
    //     :
    // );
    // kprint("hi!!!;D");

    // uint8_t* a = kalloc(5000);
    // kprint_hex((uint32_t) a);
    // kprint_hex(1/0);
    // setIDTHandler(0, )
    initInterruptHandlers();
    gdtFlush(); // move gdt to higher half
    // kprint_hex(1/0);
    // kprint_hex((uint32_t) *(uint8_t*) 0x12345567);
    kprint("hi");

    // kprint_hex(*(uint32_t*)(0x0000000)); // test page fault, works
    setPitPeriodic(1193); // fire (around) every  10 ms
    videoMemory = (volatile char*) vmmAllocatePhysicalRange(0xB8000, 4000);

    // reaload cr3
    // uint32_t* a = (uint32_t*) kalloc(1000);
    // kprint_hex((uint32_t) a);
    // kprint_hex();


    // kprint("hi");
    
    sleep(2000);
    kprint("1seclater");
    // sleep(10000);
    // kprint("10seclater");
    // kprint("esp: ");
    // uint32_t esp;
    // asm volatile (
    //     "mov %%esp, %%eax\n\t"
    //     "mov %%eax, %0\n\t"
    //     : "=m" (esp)
    //     : 
    //     : "eax"
    // );
    // kprint_hex(esp);
    // uint32_t* a = (uint32_t*)kalloc(10000);

    

    while (1) {}

}