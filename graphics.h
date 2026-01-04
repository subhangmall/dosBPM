#ifndef GRAPHICS_H
#define GRAPHICS_H

#include <stdbool.h>

void initVideoMode();
void draw(unsigned short x, unsigned short y, unsigned char color);
void drawBackground();
void drawRectangle(unsigned short x, unsigned short y, unsigned short width, unsigned short height, unsigned char color);
bool checkSVGACompatibility();
void write(unsigned char message[], unsigned char color);
void writeWithBackground(unsigned char message[], unsigned char fgColor, unsigned char bgColor);

#endif // GRAPHICS_H