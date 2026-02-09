// real mode graphicsInit.c

#include <stdint.h>
#include <stdbool.h>
#include <i86.h>
#include <malloc.h>

#pragma pack(push, 1)
struct VbeInfoBlock {
    char vbeSignature[4];
    uint16_t vbeVersion;
    uint16_t oemStringPtr[2];
    uint8_t capabilities[4];
    uint16_t videoModePtr[2]; // ptr to modes
    uint16_t totalMemory;
    uint8_t reserved[492];
};
#pragma pack(pop)

uint16_t* modes;

bool checkVGACompatibility() {
    uint16_t axAfter;

    asm volatile (
        "mov 0x4F00, %%ax\n\t"
        "int 0x10\n\t"
        "mov %%ax, %0"
        : "=r" (axAfter)
        :
        : "ax"
    );

    return (axAfter == 0x004F);
}

struct VbeInfoBlock *v;

// check if SVGA supported and if it is initialize the VgaInfoBlock
bool getSVGACompatibility() {
    uint16_t axOutput;
    v = calloc(512, 1);
    unsigned short segment = (unsigned short)((unsigned long)&v >> 16);
    unsigned short offset = (unsigned short)((unsigned long)&v & 0xFFFF);
    
    // load SVGA modes into v
    asm volatile (
        "mov 0x4F00, %%ax\n\t"
        "mov %1, %%es\n\t"
        "mov %2, %%di\n\t"
        "int $0x10\n\t"
        "mov %%ax, %0"
        : "=r" (axOutput)
        : "r" (segment), "r" (offset)
        : "ax", "di", "memory"
    );

    // if didn't return 0x004F, it means that it's not supported
    if (!(axOutput == 0x004F)) {
        return false;
    }
    return true;
}

// WORK ON LATER: FIGURE OUT A WAY TO SETUP A FAR POINTER IN GCC
bool initSVGAModes() {
    if (v == NULL && !getSVGACompatibility()) {
        return false;
    }

    uint32_t linearAddress = (v->videoModePtr[1] << 16) + v->videoModePtr[0];  
    modes = linearAddress;
}


void initVideoMode(unsigned char pal[16][3]) {
    asm volatile (
        "mov $0x00, %%ah\n\t" // Set video mode
        "mov $0x12, %%al\n\t" // 640x200 pixel graphics, 16 colors
        "int $0x10" // Call BIOS video interrupt
        :
        :
        : "ax"
    );
    
    setPalette(pal);
} 