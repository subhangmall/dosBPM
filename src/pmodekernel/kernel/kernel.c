#include "./kmemmgt.h"
#include "./logging.h"

void kentry(void) {
    kclear();

    initMem();

    kprint("Paging enabled!\n");


    kprint("Kalloc start \n");
    char* j = (char*) kalloc(50000);
    // kprint()
    kprint("Kalloc end \n");
    // kprint_hex((uint32_t) j);
    // kprint_hex(*(j + 4088));

    kprint("\n");
    kprint("Virtual address: ");
    kprint_hex((uint32_t) j);
    kprint("\nPhysical address: ");
    kprint_hex(virtToPhysAddr((uint32_t) j));

    // kprint("\n");
    // kprint("Virtual address C: ");
    // kprint_hex(0xC0000001);
    // kprint("\nPhysical address C: ");
    // kprint_hex(virtToPhysAddr(0xC0000001));

    // test kalloc (works :D)
    for (int i = 0; i < 5000; i++) {
        j[i] = 'A';
        kprint_hex(j[i]);
    }


    // kprint("Variable j declared and initialized!\n");

    free(j);

    // kprint("Variable j freed!\n");

    // video_memory[2] = 'J'; 
    
    // video_memory[3] = 0x0F;

    // // j
    // video_memory[2] = 'J'; 
    
    // video_memory[3] = 0x0F;


    // // k
    // video_memory[4] = 'P'; 
    
    // video_memory[5] = 0x0F;

    // Halt the CPU so it doesn't execute garbage memory
    uint8_t* vRAM = allocatePhysicalRange(0xB8000, 100);
    // vRAM[0] = '1';
    // vRAM[1] = 0xFF; 

    while(1) {
        __asm volatile ("hlt");
    }
}