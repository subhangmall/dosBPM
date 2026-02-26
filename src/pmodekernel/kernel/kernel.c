#include "./kmemmgt.h"
#include "./logging.h"

void kentry(void) {
    kclear();

    // kprint("Kernel Loaded Into Memory!\n");

    initMem();

    kprint("Paging enabled!\n");


    char* j = (char*) kalloc(5000);

    kprint("\n");
    kprint("Virtual address: ");
    kprint_hex((uint32_t) j);
    kprint("\nPhysical address: ");
    kprint_hex(virtToPhysAddr((uint32_t) j));

    // premove((uint32_t) j);

    // test kalloc (works :D)
    for (int i = 0; i < 5000; i++) {
        j[i] = 'A';
    }

    kprint("Variable j declared and initialized!\n");

    free(j);

    kprint("Variable j freed!\n");

    // video_memory[2] = 'J'; 
    
    // video_memory[3] = 0x0F;

    // // j
    // video_memory[2] = 'J'; 
    
    // video_memory[3] = 0x0F;


    // // k
    // video_memory[4] = 'P'; 
    
    // video_memory[5] = 0x0F;

    // Halt the CPU so it doesn't execute garbage memory
    while(1) {
        __asm volatile ("hlt");
    }
}