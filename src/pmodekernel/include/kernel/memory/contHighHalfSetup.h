#include <stdint.h>

#ifndef CONTHIGHHALFSETUP_H
#define CONTHIGHHALFSETUP_H

void continuedHigherHalfMemSetup(uint32_t e820LenAddr, uint32_t e820StartAddress, uint32_t firstKernelPageTableAddr, uint32_t kernelPageDirectoryAddr);

#endif