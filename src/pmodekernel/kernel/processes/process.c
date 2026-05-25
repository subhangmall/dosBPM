#include <stdbool.h>
#include <stdint.h>

struct ProcessAttributeStruct {
    uint32_t procID;
    struct ProcessAttributeStruct* children;
    struct ProcessAttributeStruct* next;
    struct ProcessAttributeStruct* last;
    
};