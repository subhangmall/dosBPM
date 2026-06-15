#include <stdbool.h>
#include <stdint.h>
#include <kernel/interrupts/intrStructs.h>

struct ProcessAttributeStruct {
    uint32_t procID;
    
    // WILL NOT IMPLEMENT
    // // children
    // struct ProcessAttributeStruct* firstChild;

    // // siblings
    // struct ProcessAttributeStruct* prevSibling;
    // struct ProcessAttributeStruct* nextSibling;

    // runqueue
    struct ProcessAttributeStruct* nextInQueue;
    struct ProcessAttributeStruct* prevInQueue;

    // flags
    bool kernel;
    bool v8086;

    // integral information per process
    uint32_t pageDirectoryPhysAddr;
    uint32_t kernelStackTop;
    struct InterruptStackFrame* context;
};