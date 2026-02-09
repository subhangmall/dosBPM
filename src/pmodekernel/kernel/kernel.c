// main event loop

// #include "../pmodedrv/vga.h"
// #include "memmgt.h"
// #include "idt.h"


// entry point for kernel, initialization
void kentry(void) {
    // initMem();
    // initIDT();
    volatile char* video_memory = (volatile char*) 0xB8000;

    // Write 'H' to the top-left corner
    video_memory[0] = 'H'; 
    
    // Write the color attribute (0x0F is white text on black background)
    video_memory[1] = 0x0F;

    // Halt the CPU so it doesn't execute garbage memory
    while(1) {
        __asm__("hlt");
    }
}