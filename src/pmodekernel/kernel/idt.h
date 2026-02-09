#include <stdint.h>

#ifndef IDT_H
#define IDT_H

void initIDT();
void setIDTValue(uint8_t intNum, uint32_t address, uint8_t attr, uint16_t selector);

#endif
