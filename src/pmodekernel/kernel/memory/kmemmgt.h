#include <stdint.h>
#include <stdbool.h>

#ifndef KMEMMGT_H
#define KMEMMGT_H



void continuedHigherHalfMemSetup(uint32_t firstKernelPageTableAddr, uint32_t kernelPageDirectoryAddr);
void* kalloc(uint32_t numBytes);
// void* kcalloc(uint32_t numBytes);
bool addPage(uint32_t vAddr, bool shouldZero);
void free(void* ptr);
void premove(uint32_t vAddr);
uint32_t virtToPhysAddr(uint32_t vAddr);
void* allocatePhysicalRange(uint32_t physAddr, uint32_t len);
void pmmSet(uint32_t physAddr, bool val);
bool pmmGet(uint32_t physAddr);

#endif