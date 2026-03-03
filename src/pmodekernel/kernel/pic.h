#include <stdint.h>

#ifndef PIC_H
#define PIC_H

void initPIC(uint8_t masterOffset, uint8_t slaveOffset);
void disablePIC(void);

#endif