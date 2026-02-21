#include "./kmemmgt.h"

void kentry(void) {
    volatile char* video_memory = (volatile char*) 0xB8000;
    // h
    video_memory[0] = 'H'; 
    
    video_memory[1] = 0x0F;

    initMem();

    char* j = (char*) kalloc(5000);

    // test kalloc (works :D)
    for (int i = 0; i < 5000; i++) {
        j[i] = 'A';
    }

    // j
    video_memory[2] = 'J'; 
    
    video_memory[3] = 0x0F;


    // k
    video_memory[4] = 'P'; 
    
    video_memory[5] = 0x0F;

    // Halt the CPU so it doesn't execute garbage memory
    while(1) {
        __asm volatile ("hlt");
    }
}