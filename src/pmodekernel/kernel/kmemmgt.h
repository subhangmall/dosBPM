#include <stdint.h>
#include <stdbool.h>

#ifndef KMEMMGT_H
#define KMEMMGT_H

void initMem();
void* kalloc(uint32_t numBytes);
void* kcalloc(uint32_t numBytes);
bool addPage(uint32_t vAddr, bool shouldZero);
void free(void* ptr);
void premove(uint32_t vAddr);
uint32_t virtToPhysAddr(uint32_t vAddr);

#endif