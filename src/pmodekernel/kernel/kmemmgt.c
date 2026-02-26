// paging

// SETUP E820 MAPPING, FIX FIRST 4MB MAPPING

#include <stdint.h>
#include <stdbool.h>
#include "./logging.h"

#define VMM_PRESENT    0x1
#define VMM_WRITABLE   0x2
#define VMM_USER       0x4
#define VMM_MMIO       0x8

#define PAGE_SIZE 4096
#define RECURSIVE_PD_ADDR 0xFFC00000
#define RECURSIVE_PD ((struct PageDirectoryEntry*) RECURSIVE_PD_ADDR) // where Page Directory Table appears in memory b/c the last idx of the PDT refers back to the start of the PDT
#define NULL ((void*)0)
#define MMIO_BASE 0xE0000000
#define MMIO_SIZE 0x10000000
#define AVAILABLE_MEM_E820 1
#define MAX_REGIONS 1024
#define PMM_UNAVAILABLE true
#define PMM_AVAILABLE false

static uint32_t mmioNextFree = MMIO_BASE;

uint32_t findNextPageStart();
void* kalloc(uint32_t numBytes);
void* kcalloc(uint32_t numBytes);
bool addPage(uint32_t vAddr, bool shouldZero);
void free(void* ptr);
bool allocatePage(uint32_t vAddr, uint32_t physAddr, uint8_t flags);
void zeroPage(uint32_t vAddr);

#pragma pack(push, 1)

struct KallocHeader {
    uint32_t size;
    bool isFree;
    struct KallocHeader* next;
};

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

struct E280Entry {
    uint64_t baseAddr;
    uint64_t len;
    uint32_t type; 
};

struct UsableRegion {
    uint64_t base;
    uint64_t length;
};

#pragma pack(pop)

__attribute__((aligned(PAGE_SIZE)))
struct PageDirectoryEntry kernelPageDirectory[PAGE_SIZE/sizeof(struct PageDirectoryEntry)];
__attribute__((aligned(PAGE_SIZE)))
struct PageTableEntry firstKernelPageTable[PAGE_SIZE/sizeof(struct PageTableEntry)];

static struct UsableRegion usableRegions[MAX_REGIONS];
static uint8_t usableRegionsCurrentLength = 0;

static uint8_t physicalPageRecord[0xFFFFFFFF/PAGE_SIZE/8];

struct KallocHeader* heapStart = NULL;
uint32_t nextVAddrToMap = 0x40000000;

static uint32_t nextFreePageFramePhysicalStartingAddress = 4194304 - PAGE_SIZE;
static uint32_t maxMemory = 4294967296 - 1;
static uint32_t currentRegion = 0;

void pmmSet(uint32_t physAddr, bool val) {
    if (val) {
        physicalPageRecord[physAddr >> 17] |= 0b00000001 << ((physAddr & 0b00000000000000000111000000000000) >> 12);
    } else { 
        physicalPageRecord[physAddr >> 17] &= ~(0b00000001 << ((physAddr & 0b00000000000000000111000000000000) >> 12));
    }
}

bool pmmGet(uint32_t physAddr) {
    return physicalPageRecord[physAddr >> 17] & (0b00000001 << ((physAddr & 0b00000000000000000111000000000000) >> 12));
}

void sortRegions() {
    for (int i = 0; i < usableRegionsCurrentLength; i++) {
        for (int j = 0; j < usableRegionsCurrentLength -i - 1; j++) {
            if (usableRegions[j].base > usableRegions[j+1].base) {
                struct UsableRegion tmp = usableRegions[j];
                usableRegions[j] = usableRegions[j+1];
                usableRegions[j+1] = tmp;
            }
        }
    }
}

void mergeRegions() {
    for (int i = 0; i < usableRegionsCurrentLength - 1; i++) {

        uint64_t currentEnd = usableRegions[i].base + usableRegions[i].length;

        uint64_t nextEnd = usableRegions[i+1].base + usableRegions[i+1].length;

        if (currentEnd >= usableRegions[i+1].base) {
            uint64_t newEnd = currentEnd > nextEnd ? currentEnd : nextEnd;

            usableRegions[i].length = newEnd - usableRegions[i].base;

            for (int j = i + 1; j < usableRegionsCurrentLength - 1; j++) {
                usableRegions[j] = usableRegions[j+1];
            }

            usableRegionsCurrentLength--;
            i--;  // re check merged region
        }
    }
}

void parseE820Output() {
    // setup frame allocator
    for (uint32_t i = 0; i < 0xFFFFFFFF/PAGE_SIZE/8; i++) {
        physicalPageRecord[i] = 0xFF; // means all 8 bits are used
    }

    uint16_t* listLength = (uint16_t*) 0x00008000; // where the length of the list is stored by the assembler;
    // 0x5000 is where the entries are stored, each 24 bytes

    // kprint("List length: ");
    // kprint_hex((uint32_t) *listLength);
    // kprint("\n");

    for (uint32_t i = 0; i < *listLength; i++) {
        if (usableRegionsCurrentLength == MAX_REGIONS) {
            break;
        }
        
        struct E280Entry* entry = (struct E280Entry*) (i * 24 + 0x5000);

        if (entry->type == AVAILABLE_MEM_E820) {
            struct UsableRegion region = {
                .base = entry->baseAddr,
                .length = entry->len
            };

            usableRegions[usableRegionsCurrentLength++] = region;
        }
    }

    // for (int i = 0; i < usableRegionsCurrentLength; i++) {
    //     kprint("Start range unsorted: "); 
    //     kprint_hex64(usableRegions[i].base);
    //     kprint("\nEnd range unsorted: ");
    //     kprint_hex64(usableRegions[i].length + usableRegions[i].base);
    //     // kprint("\n");
    // }

    // kprint("\n");

    sortRegions();

    // for (int i = 0; i < usableRegionsCurrentLength; i++) {
    //     kprint("Start range sorted: "); 
    //     kprint_hex64(usableRegions[i].base);
    //     kprint("\nEnd range sorted: ");
    //     kprint_hex64(usableRegions[i].length + usableRegions[i].base);
    //     kprint("\n");
    // }

    // kprint("\n");

    mergeRegions();

    // for (int i = 0; i < usableRegionsCurrentLength; i++) {
    //     kprint("Start range: "); 
    //     kprint_hex64(usableRegions[i].base);
    //     kprint("\nEnd range: ");
    //     kprint_hex64(usableRegions[i].length + usableRegions[i].base);
    //     kprint("\n");
    // }

    // kprint("Usable list length: ");
    // kprint_hex(usableRegionsCurrentLength);
    // kprint("\n");

    for (int i = 0; i < usableRegionsCurrentLength; i++) {
        for (int j = usableRegions[i].base | 0x00000FFF; j < usableRegions[i].base + usableRegions[i].length; j+=4096) {
            pmmSet(j, PMM_AVAILABLE);
        }
    }
}




void initMem() {
    parseE820Output();

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

    for (int i = 0; i < PAGE_SIZE/sizeof(struct PageTableEntry); i++) {
        kernelPageDirectory[i] = pde;
        
        pte.pageAddress = i;
        
        firstKernelPageTable[i] = pte;
    }

    struct PageDirectoryEntry firstPage =  {
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

    pmmSet((uint32_t) &firstKernelPageTable, PMM_UNAVAILABLE);

    // points to page directory itself
    struct PageDirectoryEntry lastPage =  {
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
    };

    pmmSet((uint32_t) &kernelPageDirectory, PMM_UNAVAILABLE);

    // points to first page
    kernelPageDirectory[0] = firstPage;
    kernelPageDirectory[1023] = lastPage;
    
    // REMEMBER TO FLUSH TLB PER CHANGE

    // move addr into cr3 register and flip the right bit on cr0 to enable paging
    uint32_t kPDAddress = (uint32_t)&kernelPageDirectory;
    asm volatile (
        "mov %0, %%cr3\n\t"
        "mov %%cr0, %%eax\n\t"
        "or $0x80000000, %%eax\n\t"
        "mov %%eax, %%cr0\n\t"
        "jmp 1f\n\t" // to clear prefetched instructions
        "1:"
        :
        : "r" (kPDAddress)
        : "eax"
    );
}

bool addPage(uint32_t vAddr, bool shouldZero) {
    // uint16_t ptdIdx = (uint16_t)((vAddr >> 22)&0b1111111111);

    // if (kernelPageDirectory[ptdIdx].present == 1) {
    //     // page table present
    //     struct PageTableEntry* pte = (struct PageTableEntry*)(RECURSIVE_PD_ADDR + ((vAddr >> 12) * 4)); 
        
    //     if (pte->present == 0) {
    //         // page not allocated yet
    //         uint32_t pagePhysicalAddr = findNextPageStart();
    //         if (pagePhysicalAddr == 0xFFFFFFFF) return false; // out of memory

    //         preventVMMFromAllocatingPage(pagePhysicalAddr);

    //         struct PageTableEntry pte = { 
    //             .present = 1,
    //             .rw = 1,
    //             .user = 0,
    //             .pwt = 0,
    //             .pcd = 0,
    //             .accessed = 0,
    //             .dirty = 0,
    //             .pageTableAttrIdx = 0,
    //             .globalPage = 0,
    //             .available = 0,
    //             .pageAddress = (pagePhysicalAddr >> 12)
    //         };
            
    //         // FIX LINE
    //         uint32_t pteAddress = RECURSIVE_PD_ADDR + ((vAddr >> 12) * 4);
    //         struct PageTableEntry* ptePtr = (struct PageTableEntry*)pteAddress;
                
    //         *ptePtr = pte;



    //         // 0xFFC0000 refers to index 1023, which just refers back to itself
    //         // vaddr >> 12 * 4 is the page table number + page directory number
                
    //         asm volatile (
    //         "invlpg (%0)"
    //         :
    //         : "r" (vAddr)
    //         : "memory"
    //         );
            
    //         if (shouldZero) {
    //             vAddr &= 0b11111111111111111111000000000000;
    //             uint8_t* pageBytes = (uint8_t*) vAddr;
    //             for (int i = 0; i < 4096; i++) {
    //                 pageBytes[i] = 0x00;
    //             }
    //         }
            
    //         return true;

    //     } else {
    //         if (shouldZero) {
    //             vAddr &= 0b11111111111111111111000000000000;
    //             uint8_t* pageBytes = (uint8_t*) vAddr;
    //             for (int i = 0; i < 4096; i++) {
    //                 pageBytes[i] = 0x00;
    //             }
    //         }
    //         return true; // page already allocated
    //     }
        
    // } else {
    //     // page table not present
    //    uint32_t pageStart = findNextPageStart();
    //    if (pageStart == 0xFFFFFFFF) return false; // no mem left

    //    struct PageDirectoryEntry new  = {
    //         .present = 1,
    //         .rw = 1,
    //         .user = 0,
    //         .pwt = 0,
    //         .pcd = 0,
    //         .accessed = 0,
    //         .reserved  = 0,
    //         .pageSize  = 0,
    //         .ignored  = 0,
    //         .available = 1,
    //         .pageAddress = pageStart >> 12
    //    };

    //    kernelPageDirectory[ptdIdx] = new;

    //    asm volatile (
    //         "mov %%cr3, %%eax\n"
    //         "mov %%eax, %%cr3\n"
    //         :
    //         :
    //         : "eax", "memory"
    //     );

    //    uint8_t *newPageTable = (uint8_t*) (RECURSIVE_PD_ADDR + (ptdIdx << 12));

    //    for (int i = 0; i < 4096; i++) {
    //         newPageTable[i] = 0x00;
    //    };

    //    return addPage(vAddr, shouldZero);
    // }

    bool result = allocatePage(vAddr, findNextPageStart(), VMM_WRITABLE);
    if (shouldZero && result) {
        zeroPage(vAddr);

        return true;
    } else {
        return result;
    }
}

// page allocation from far memory. returns physical first 20 bits of address shifted right 12 bites
uint32_t findNextPageStart() {
    // bool found = true;
    // for (int i = 0; i < usableRegionsCurrentLength; i++) {
    //     if (usableRegions[i].base <= nextFreePageFramePhysicalStartingAddress && !(4096<=(usableRegions[i].length + usableRegions[i].base - nextFreePageFramePhysicalStartingAddress))) {
    //         for (int j = i; j < usableRegionsCurrentLength; j++) {
    //             if (usableRegions[j].length >= 4096) {
    //                 nextFreePageFramePhysicalStartingAddress = usableRegions[j].base; 
    //                 break;
    //             } else if (j + 1 == usableRegionsCurrentLength) {
    //                 found = false;
    //             }
    //         }
    //         break;
    //     }
    // }

    if (nextFreePageFramePhysicalStartingAddress + PAGE_SIZE > maxMemory) {
        return 0xFFFFFFFF;
    } else {
        nextFreePageFramePhysicalStartingAddress += PAGE_SIZE;

        return nextFreePageFramePhysicalStartingAddress;
    }

    
}

// zero out PTE for an unused page
void premove(uint32_t vAddr) {
    // first 10 bits is Page Directory, next 10 is table index, next 12 is page offset
    // loop back to start of page directory, then set the PTE to zero
    uint32_t *PTE = (uint32_t*)((RECURSIVE_PD_ADDR) + 
        ((0b11111111111111111111000000000000 & vAddr) >> 10));
    *PTE = 0x00000000; // deactivate page in memory


    // refresh tlb
    asm volatile (
            "invlpg (%0)"
            :
            : "r" (vAddr)
            : "memory"
    );
}

// kernel allocation, for drivers and such
void* kalloc(uint32_t numBytes) {
    if (heapStart == NULL) {
        addPage(nextVAddrToMap, false);
        struct KallocHeader* head = (struct KallocHeader*)nextVAddrToMap;
        head->size = 4096-sizeof(struct KallocHeader);
        head->isFree = true;
        head->next = NULL;
        heapStart = head;
    } 
    struct KallocHeader* current = heapStart;
    while (current != NULL) {
        if (!current->isFree || current->size < numBytes) {
            // the current segment isn't going to work for the size we're trying to allocate
            if (current->next == NULL) {
                // if there is no next one and we couldn't find a good one before, create a new next one and allocate memory to it
                uint32_t nextAddr = (uint32_t) current;
                nextAddr += sizeof(struct KallocHeader);
                nextAddr += current->size;
                
                // check if there is enough memory free to allocate pages between nextAddr and the end of this hypothetical kalloc
                for (int i = nextAddr & 0b11111111111111111111000000000000; i < ((nextAddr + sizeof(struct KallocHeader) + numBytes) | 0b00000000000000000000111111111111); i+=PAGE_SIZE) {
                    if (!addPage(i, false)) return NULL;
                }
                struct KallocHeader* nextHeader = (struct KallocHeader*)nextAddr;
                *nextHeader = (struct KallocHeader) {
                    .size = numBytes,
                    .isFree = false,
                    .next = NULL
                };
                current->next = nextHeader;
                nextAddr += sizeof(struct KallocHeader);
                return (void*)nextAddr;
            }
        } else {
            // we find a segment that can fit
            uint32_t kallocSegmentToReturn = (uint32_t) current;
            current->isFree = false;
            kallocSegmentToReturn += sizeof(struct KallocHeader);
            return (void*) kallocSegmentToReturn;
        }
        current = current->next;
    }
}

// zeroes out memory as well
void* kcalloc(uint32_t numBytes) {
    if (heapStart == NULL) {
        addPage(nextVAddrToMap, true);
        struct KallocHeader* head = (struct KallocHeader*)nextVAddrToMap;
        head->size = 4096-sizeof(struct KallocHeader);
        head->isFree = true;
        head->next = NULL;
        heapStart = head;
    } 
    struct KallocHeader* current = heapStart;
    while (current != NULL) {
        if (!current->isFree || current->size < numBytes) {
            // the current segment isn't going to work for the size we're trying to allocate
            if (current->next == NULL) {
                // if there is no next one and we couldn't find a good one before, create a new next one and allocate memory to it
                uint32_t nextAddr = (uint32_t) current;
                nextAddr += sizeof(struct KallocHeader);
                nextAddr += current->size;
                
                // check if there is enough memory free to allocate pages between nextAddr and the end of this hypothetical kalloc
                for (int i = nextAddr & 0b11111111111111111111000000000000; i < ((nextAddr + sizeof(struct KallocHeader) + numBytes) | 0b00000000000000000000111111111111); i+=PAGE_SIZE) {
                    if (!addPage(i, false)) return NULL;
                }
                struct KallocHeader* nextHeader = (struct KallocHeader*)nextAddr;
                *nextHeader = (struct KallocHeader) {
                    .size = numBytes,
                    .isFree = false,
                    .next = NULL
                };
                current->next = nextHeader;
                nextAddr += sizeof(struct KallocHeader);
                return (void*)nextAddr;
            }
        } else {
            // we find a segment that can fit
            uint32_t kallocSegmentToReturn = (uint32_t) current;
            current->isFree = false;
            kallocSegmentToReturn += sizeof(struct KallocHeader);
            return (void*) kallocSegmentToReturn;
        }
        current = current->next;
    }
}

void free(void* ptr) {
    if (ptr == NULL) return;

    uint32_t addr = (uint32_t) ptr;
    addr -= sizeof(struct KallocHeader);
    struct KallocHeader* kh = (struct KallocHeader*)addr;
    kh->isFree = true;
    while (kh->next != NULL && kh->next->isFree) {
        kh->size += kh->next->size + sizeof(struct KallocHeader);
        kh->next = kh->next->next;
    }
}

uint32_t virtToPhysAddr(uint32_t vAddr) {
    uint32_t *PTE = (uint32_t*)((RECURSIVE_PD_ADDR) + 
        ((0xFFFFF000 & vAddr) >> 10));
    struct PageTableEntry *pte = (struct PageTableEntry*) PTE;
    // << 12
    return (((pte->pageAddress) << 12) + (0x00000FFF & vAddr));
}

bool createNewPageTable(uint32_t vAddr) {
    uint16_t ptdIdx = (uint16_t)((vAddr >> 22)&0b1111111111);

    uint32_t pageStart = findNextPageStart();

    if (pageStart == 0xFFFFFFFF) return false; // no mem left

    struct PageDirectoryEntry new  = {
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
        .pageAddress = pageStart >> 12
    };

    kernelPageDirectory[ptdIdx] = new;

    asm volatile (
        "mov %%cr3, %%eax\n"
        "mov %%eax, %%cr3\n"
        :
        :
        : "eax", "memory"
    );

    uint8_t *newPageTable = (uint8_t*) (RECURSIVE_PD_ADDR + (ptdIdx << 12));

    for (int i = 0; i < 4096; i++) {
        newPageTable[i] = 0x00;
    };

    return true;
}

void zeroPage(uint32_t vAddr) {
    vAddr &= 0b11111111111111111111000000000000;
    uint8_t* pageBytes = (uint8_t*) vAddr;
    for (int i = 0; i < 4096; i++) {
        pageBytes[i] = 0x00;
    }
}

bool allocatePage(uint32_t vAddr, uint32_t physAddr, uint8_t flags) {
    uint16_t ptdIdx = (uint16_t)((vAddr >> 22)&0b1111111111);

    if (kernelPageDirectory[ptdIdx].present == 1) {
        // page table present
        struct PageTableEntry* pte = (struct PageTableEntry*)(RECURSIVE_PD_ADDR + ((vAddr >> 12) * 4)); 
        
        if (pte->present == 0) {
            // page not allocated yet
            uint32_t pagePhysicalAddr = findNextPageStart();
            if (pagePhysicalAddr == 0xFFFFFFFF) return false; // out of memory

            pmmSet(pagePhysicalAddr, PMM_UNAVAILABLE);

            struct PageTableEntry pte = { 
                .present = 0,
                .rw = 0,
                .user = 0,
                .pwt = 0,
                .pcd = 0,
                .accessed = 0,
                .dirty = 0,
                .pageTableAttrIdx = 0,
                .globalPage = 0,
                .available = 0,
                .pageAddress = (pagePhysicalAddr >> 12)
            };

            if (flags == VMM_PRESENT) {
                pte.present = 1;
            }

            if (flags == VMM_WRITABLE) {
                pte.present = 1;
                pte.rw = 1;
            }

            if (flags == VMM_USER) {
                pte.present = 1;
                pte.rw = 1;
                pte.user = 1;
            }

            if (flags == VMM_MMIO) {
                pte.present = 1;
                pte.rw = 1;
                pte.pcd = 1;
                pte.pwt = 1;
            }
            
            uint32_t pteAddress = RECURSIVE_PD_ADDR + ((vAddr >> 12) * 4);
            struct PageTableEntry* ptePtr = (struct PageTableEntry*)pteAddress;
                
            *ptePtr = pte;

            // FIX LATER
            // pmmSet(pagePhysicalAddr, PMM_AVAILABLE);

            // 0xFFC0000 refers to index 1023, which just refers back to itself
            // vaddr >> 12 * 4 is the page table number + page directory number
                
            asm volatile (
            "invlpg (%0)"
            :
            : "r" (vAddr)
            : "memory"
            );
            
            // if (shouldZero) {
            //     vAddr &= 0b11111111111111111111000000000000;
            //     uint8_t* pageBytes = (uint8_t*) vAddr;
            //     for (int i = 0; i < 4096; i++) {
            //         pageBytes[i] = 0x00;
            //     }
            // }
            
            return true;

        } else {
            // if (shouldZero) {
            //     vAddr &= 0b11111111111111111111000000000000;
            //     uint8_t* pageBytes = (uint8_t*) vAddr;
            //     for (int i = 0; i < 4096; i++) {
            //         pageBytes[i] = 0x00;
            //     }
            // }
            return true; // page already allocated
        }
        
    } else {
        // page table not present
       if (!createNewPageTable(vAddr)) return false;

       return allocatePage(vAddr, physAddr, flags);
    }
}



void deallocateVirtMemRange(uint32_t vAddr, uint32_t len) {

}