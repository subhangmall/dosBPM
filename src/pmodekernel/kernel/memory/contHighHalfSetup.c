#include <stdint.h>
#include <kernel/memory/e820.h>
#include <kernel/memory/pmm.h>
#include <kernel.h>
#include <kernel/memory.h>
#include <kernel/logging.h>

void continuedHigherHalfMemSetup(uint32_t e820LenAddr, uint32_t e820StartAddress, uint32_t firstKernelPageTableAddr, uint32_t kernelPageDirectoryAddr) {
    parseE820Output(e820LenAddr, e820StartAddress);
    kprint("len addr:");
    kprint_hex(e820LenAddr);
    kprint("\nstart addr:");
    kprint_hex(e820StartAddress);

    // *((uint8_t*)0xB8002) = 'p';
    // *((uint8_t*)0xB8003) = 0x0F;

    pmmSet(firstKernelPageTableAddr, PMM_UNAVAILABLE);
    pmmSet(kernelPageDirectoryAddr, PMM_UNAVAILABLE);
    for (int i = 0; i < 0x400000; i+= PAGE_SIZE) {
        pmmSet(i, PMM_UNAVAILABLE);
    }

    // *((uint8_t*)0xB8002) = 'E';
    // *((uint8_t*)0xB8003) = 0x0F;
}

