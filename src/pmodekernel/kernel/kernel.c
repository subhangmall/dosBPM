// #include "./kmemmgt.h"
// #include "./logging.h"
// #include "./idt.h"
// #include "./pic.h"
// #include "./basicInterruptHandlers.h"
#include <stdint.h>
#include <stdbool.h>

// volatile char* videoMemory;

#define VMM_PRESENT    0x1
#define VMM_WRITABLE   0x2
#define VMM_USER       0x4
#define VMM_MMIO       0x8

#define PAGE_SIZE 4096
#define RECURSIVE_PD_ADDR 0xFFC00000
#define RECURSIVE_PD ((struct PageDirectoryEntry*) RECURSIVE_PD_ADDR) // where Page Directory Table appears in memory b/c the last idx of the PDT refers back to the start of the PDT
#define NULL ((void*)0)
#define MMIO_VIRTUAL_SPACE_BASE 0xE0000000
#define MMIO_VIRTUAL_SPACE_SIZE 0x10000000
#define AVAILABLE_MEM_E820 1
#define MAX_REGIONS 1024
#define PMM_UNAVAILABLE true
#define PMM_AVAILABLE false
#define ALIGN_DOWN(x) ((x) & ~(PAGE_SIZE - 1))
#define ALIGN_UP(x) (((x) + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1))

#define KERNEL_STACK_VADDR 0xC0100000
#define KERNEL_STACK_SIZE  0x4000

void highHalfEntry(void);

#pragma pack(push, 1)

struct PageDirectoryEntry {
    uint32_t present : 1;
    uint32_t rw : 1;
    uint32_t user : 1;
    uint32_t pwt : 1;
    uint32_t pcd : 1;
    uint32_t accessed : 1;
    uint32_t reserved : 1;
    uint32_t pageSize : 1;
    uint32_t ignored : 1;
    uint32_t available : 3;
    uint32_t pageAddress : 20;
};

struct PageTableEntry {
    uint32_t present : 1;
    uint32_t rw : 1;
    uint32_t user : 1;
    uint32_t pwt : 1;
    uint32_t pcd : 1;
    uint32_t accessed : 1;
    uint32_t dirty : 1;
    uint32_t pageTableAttrIdx : 1;
    uint32_t globalPage : 1;
    uint32_t available : 3;
    uint32_t pageAddress : 20;
};

#pragma pack(pop)


// __attribute__((section(".boot.data")))
// static uint8_t physicalPageRecord[0xFFFFFFFF/PAGE_SIZE/8];

__attribute__((aligned(PAGE_SIZE)))
__attribute__((section(".boot.data")))
struct PageDirectoryEntry kernelPageDirectory[PAGE_SIZE/sizeof(struct PageDirectoryEntry)];

__attribute__((aligned(PAGE_SIZE)))
__attribute__((section(".boot.data")))
struct PageTableEntry firstKernelPageTable[PAGE_SIZE/sizeof(struct PageTableEntry)];

__attribute__((section(".boot.data")))
static int cursor = 0;

__attribute__((section(".boot.data")))
volatile char* videoMemory = (volatile char*) 0xB8000;

__attribute__((section(".boot"))) void kentry(void);
__attribute__((section(".boot"))) void kprint_hex(uint32_t n);

__attribute__((section(".boot"))) void kentry(void) {
    // kclear();
    *((uint8_t*)0xB8000) = 'A';
    *((uint8_t*)0xB8001) = 0x0F;

    
    struct PageDirectoryEntry pde = {
        .present = 0,
        .rw = 0,
        .user = 0,
        .pwt = 0,
        .pcd = 0,
        .accessed = 0,
        .reserved  = 0,
        .pageSize  = 0,
        .ignored  = 0,
        .available = 0,
        .pageAddress = 0
    };

    struct PageTableEntry pte = { 
        .present = 1,
        .rw = 1,
        .user = 0,
        .pwt = 0,
        .pcd = 0,
        .accessed = 0,
        .dirty = 0,
        .pageTableAttrIdx = 0,
        .globalPage = 0,
        .available = 0,
        .pageAddress = 0
    };

    // // pmmSet((uint32_t) &kernelPageDirectory, PMM_UNAVAILABLE);

    *((uint8_t*)0xB8002) = 'B';
    *((uint8_t*)0xB8003) = 0x0F;

    for (int i = 0; i < PAGE_SIZE/sizeof(struct PageTableEntry); i++) {
        kernelPageDirectory[i] = pde; // zero PDEs
        
        pte.pageAddress = i;
        
        firstKernelPageTable[i] = pte; // setup PTE for first 4 megabytes
    }

    *((uint8_t*)0xB8004) = 'C';
    *((uint8_t*)0xB8005) = 0x0F;

    struct PageDirectoryEntry firstPageDirectoryEntry =  {
        .present = 1,
        .rw = 1,
        .user = 0,
        .pwt = 0,
        .pcd = 0,
        .accessed = 0,
        .reserved  = 0,
        .pageSize  = 0,
        .ignored  = 0,
        .available = 1,
        .pageAddress = ((uint32_t) &firstKernelPageTable) >> 12
    };

    // // pmmSet((uint32_t) &firstKernelPageTable, PMM_UNAVAILABLE);

    *((uint8_t*)0xB8006) = 'D';
    *((uint8_t*)0xB8007) = 0x0F;

    // points to page directory itself
    struct PageDirectoryEntry loopBack =  {
        .present = 1,
        .rw = 1,
        .user = 0,
        .pwt = 0,
        .pcd = 0,
        .accessed = 0,
        .reserved  = 0,
        .pageSize  = 0,
        .ignored  = 0,
        .available = 1,
        .pageAddress = ((uint32_t) &kernelPageDirectory) >> 12
        // .pageAddress = 0
    };

    *((uint8_t*)0xB8008) = 'E';
    *((uint8_t*)0xB8009) = 0x0F;

    // // pmmSet((uint32_t) &kernelPageDirectory, PMM_UNAVAILABLE);

    // // points to first page
    kernelPageDirectory[0] = firstPageDirectoryEntry;
    kernelPageDirectory[0xC0000000 >> 22] = firstPageDirectoryEntry; // address 0xC0000000
    kernelPageDirectory[1023] = loopBack;

    *((uint8_t*)0xB800A) = 'F';
    *((uint8_t*)0xB800B) = 0x0F;
    
    // REMEMBER TO FLUSH TLB PER CHANGE
    // *((uint8_t*)0xB8002) = 'B';
    // *((uint8_t*)0xB8003) = 0x0F;

    // move addr into cr3 register and flip the right bit on cr0 to enable paging
    uint32_t kPDAddress = (uint32_t)&kernelPageDirectory;
    
    void (*highHalfEntryPTR)() = highHalfEntry;
    kprint_hex((uint32_t)highHalfEntryPTR);
    // highHalfEntryPTR += 0xC0000000;

    asm volatile (
        "mov %0, %%cr3\n\t"
        "mov %%cr0, %%eax\n\t"
        "or $0x80000000, %%eax\n\t"
        "mov %%eax, %%cr0\n\t"
        "jmp 1f\n\t" // to clear prefetched instructions
        "1:\n\t"
        "jmp *%1\n\t"        
        :
        :  "r" (kPDAddress), "r"(highHalfEntry)
        // :  "r" (kPDAddress)
        : "eax"
    );

    // highHalfEntry();

    // kprint("Paging enabled!\n");


    // kprint("Kalloc start \n");
    // char* j = (char*) kalloc(5000);
    // // kprint()
    // kprint("Kalloc end \n");
    // // kprint_hex((uint32_t) j);
    // // kprint_hex(*(j + 4088));

    // kprint("\n");
    // kprint("Virtual address: ");
    // kprint_hex((uint32_t) j);
    // kprint("\nPhysical address: ");
    // kprint_hex(virtToPhysAddr((uint32_t) j));

    // kprint("\n");
    // kprint("Virtual address C: ");
    // kprint_hex(0xC0000001);
    // kprint("\nPhysical address C: ");
    // kprint_hex(virtToPhysAddr(0xC0000001));

    // // test kalloc (works :D)
    // for (int i = 0; i < 5000; i++) {
    //     j[i] = 'A';
    //     // kprint_hex(j[i]);
    // }

    // char* b = kalloc(1000);

    // kprint("Variable j declared and initialized!\n");

    // free(j);
    // free(b);

    // char* c = kalloc(10000);

    // kprint("Variable j freed!\n");

    // // video_memory[2] = 'J'; 
    
    // // video_memory[3] = 0x0F;

    // // // j
    // // video_memory[2] = 'J'; 
    
    // // video_memory[3] = 0x0F;

    // // setIDTHandler(0xFF, (uint32_t) peepeepoopoo);
    // // // k
    // // video_memory[4] = 'P'; 
    
    // // video_memory[5] = 0x0F;

    // // Halt the CPU so it doesn't execute garbage memory
    // uint8_t* vRAM = allocatePhysicalRange(0xB8000, 100);
    // // kprint_hex(virtToPhysAddr((uint32_t) vRAM));
    
    // videoMemory = vRAM;
    // // kclear();
    // kprint("\nReloaded video memory to use virtual address \n");

    // kprint_hex((uint32_t) vRAM);

    // initIDTStructures();

    // setIDTHandler(0, (uint32_t) divByZeroException);

    // enableIDT();
    // asm volatile (
    //     "int $0x02\n\t"
    //     :
    //     :
    //     :
    // );

    // asm volatile (
    //     "int $0x00\n\t"
    //     :
    //     :
    //     :
    // );

    // asm volatile (
    //     "int $0xFF\n\t"
    //     :
    //     :
    //     :
    // );

    // kprint_hex(1/0);

    // initPIC(0x20, 0x28);

    // enableExternalInterrupts();
    // disableExternalInterrupts();

    // kprint_hex(1/0);

    // while (1) {
    //     // kprint("HI");
    // }

    // kprint_hex(1/0);

    // *((uint8_t*)0xB8004) = 'D';
    // *((uint8_t*)0xB8005) = 0x0F;

    while(1) {
        __asm volatile ("hlt");
    }
}

__attribute__((section(".boot"))) void kclear() {
    for (int i = 0; i < 80*25; i++) {
        videoMemory[i*2] = ' ';
        videoMemory[i*2 + 1] = 0x0F;
    }
    cursor = 0;
}

__attribute__((section(".boot"))) void kputc(char c) {
    if (cursor == 80 * 25) {
        kclear();
    }
    if (c == '\n') {
        cursor = (cursor / 80 + 1) * 80;
        return;
    }

    videoMemory[cursor*2] = c;
    videoMemory[cursor*2+1] = 0x0F;
    cursor++;
}

__attribute__((section(".boot"))) void kprint(const char* s) {
    while (*s)
        kputc(*s++);
}

__attribute__((section(".boot"))) void kprint_hex(uint32_t n) {
    char hexChars[] = "0123456789ABCDEF";
    char result[11];

    result[0] = '0';
    result[1] = 'x';
    result[10] = '\0';

    for (int i = 2; i <= 9; i++) {
        result[11-i] = hexChars[n & 0x0F];
        n >>= 4;
    }

    kprint(result);
}

__attribute__((section(".boot"))) void kprint_hex64(uint64_t n) {
    char hexChars[] = "0123456789ABCDEF";
    char result[19];

    result[0] = '0';
    result[1] = 'x';
    result[18] = '\0';

    for (int i = 2; i <= 17; i++) {
        result[19-i] = hexChars[n & 0x0F];
        n >>= 4;
    }

    kprint(result);
}

void highHalfEntry(void) {
    kprint("hi");

    int a = 99;
    kprint_hex((uint32_t)&a);
    kprint_hex(*(&a + 0xC0000000));

    __asm volatile (
        "hlt"
        :
        :
        :
    );
}