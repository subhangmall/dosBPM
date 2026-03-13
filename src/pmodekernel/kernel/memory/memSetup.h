#include <stdint.h>
#include <stdbool.h>

#ifndef MEMSETUP_H
#define MEMSETUP_H

void initMemory(void (*functionToJumpToAfterCompletion)());

#endif