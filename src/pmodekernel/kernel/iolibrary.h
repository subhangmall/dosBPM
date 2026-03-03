#include <stdint.h>

#ifndef IOLIBRARY_H
#define IOLIBRARY_H

void outb(uint16_t port, uint8_t val);
uint8_t inb(uint16_t port);
void ioWait(void);

#endif