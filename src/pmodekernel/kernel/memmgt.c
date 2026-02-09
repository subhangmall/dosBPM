// paging
#include <stdint.h>
#include <stdbool.h>

#define PAGE_SIZE 4096
#define RECURSIVE_PD ((struct PageDirectoryEntry*)0xFFFFF000) // where Page Directory Table appears in memory b/c the last idx of the PDT refers back to the start of the PDT
#define RECURSIVE_PT ((struct PageTableEntry*)0xFFC00000) // first page table
#define NULL ((void*)0)

uint32_t palloc();
void* kalloc(uint32_t numBytes);
bool addPage(uint32_t vAddr, bool shouldZero);
void free(void* ptr);

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

#pragma pack(pop)

__attribute__((aligned(PAGE_SIZE)))
struct PageDirectoryEntry kernelPageDirectory[PAGE_SIZE/sizeof(struct PageDirectoryEntry)];
__attribute__((aligned(PAGE_SIZE)))
struct PageTableEntry firstKernelPageTable[PAGE_SIZE/sizeof(struct PageTableEntry)];

struct KallocHeader* heapStart = NULL;
uint32_t nextVAddrToMap = 0x40000000;

static uint32_t nextFreePageFramePhysicalStartingAddress = 4194304 - PAGE_SIZE;
static uint32_t maxMemory = 4294967296 - 1;

void initMem() {
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
    uint16_t ptdIdx = (uint16_t)((vAddr >> 22)&0b1111111111);

    if (kernelPageDirectory[ptdIdx].present == 1) {
        // page table present
        struct PageTableEntry* pte = (struct PageTableEntry*)(vAddr >> 12); 
        
        if (pte->present == 0) {
            // page not allocated yet
            uint32_t pagePhysicalAddr = palloc();
            if (pagePhysicalAddr == 0xFFFFFFFF) return false; // out of memory

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
                .pageAddress = (pagePhysicalAddr >> 12)
            };

            uint32_t pteAddress = 0xFFC00000 + ((vAddr >> 12) * 4);
            struct PageTableEntry* ptePtr = (struct PageTableEntry*)pteAddress;
                
            *ptePtr = pte;



            // 0xFFC0000 refers to index 1023, which just refers back to itself
            // vaddr >> 12 * 4 is the page table number + page directory number
                
            asm volatile (
            "invlpg (%0)"
            :
            : "r" (vAddr)
            : "memory"
            );
            
            if (shouldZero) {
                vAddr &= 0b111111111111;
                uint32_t* pageBytes = (uint32_t*) vAddr;
                for (int i = 0; i < 32; i++) {
                    pageBytes[i] = 0x00000000;
                }
            }
            
            return true;

            


        } else {
            if (shouldZero) {
                vAddr &= 0b111111111111;
                uint32_t* pageBytes = (uint32_t*) vAddr;
                for (int i = 0; i < 32; i++) {
                    pageBytes[i] = 0x00000000;
                }
            }
            return true; // page already allocated
        }
        
    } else {
        // page table not present
       uint32_t pageStart = palloc();
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
            "invlpg (%0)"
            :
            : "r" (vAddr)
            : "memory"
       );
       return addPage(vAddr);
    }
}

// page allocation from far memory. returns physical first 20 bits of address shifted right 12 bites
uint32_t palloc() {
    if (nextFreePageFramePhysicalStartingAddress + PAGE_SIZE > maxMemory) {
        return 0xFFFFFFFF;
    } else {
        nextFreePageFramePhysicalStartingAddress += PAGE_SIZE;
        return nextFreePageFramePhysicalStartingAddress;
    }
}

// kernel allocation, for drivers and such
void* kalloc(uint32_t numBytes) {
    if (heapStart == NULL) {
        addPage(nextVAddrToMap);
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
                for (int i = nextAddr; i <= nextAddr + sizeof(struct KallocHeader) + numBytes; i+=PAGE_SIZE) {
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
void* kcalloc(int numBytes) {
    if (heapStart == NULL) {
        addPage(nextVAddrToMap);
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
                for (int i = nextAddr; i <= nextAddr + sizeof(struct KallocHeader) + numBytes; i+=PAGE_SIZE) {
                    if (!addPage(i, true)) return NULL;
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