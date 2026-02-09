#include <stdbool.h>

#ifndef VGAINIT_H
#define VGAINIT_H


void setPalette(unsigned char colorPalette[16][3]);
bool checkVGACompatibility();
void initVideoMode(unsigned char pal[16][3]);

#endif