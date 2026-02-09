#ifndef GRAPHICS_H
#define GRAPHICS_H

#include <stdbool.h>

void initVideoMode();
void draw(unsigned short x, unsigned short y, unsigned char color);
void drawBackground();
void drawRectangle(unsigned short x, unsigned short y, unsigned short width, unsigned short height, unsigned char color);
bool checkSVGACompatibility();
void write(unsigned char message[], unsigned char color, int x, int y);
void writeWithBackground(unsigned char message[], unsigned char fgColor, unsigned char bgColor, int x, int y);
void drawLine(unsigned short startX, unsigned short startY, unsigned short endX, unsigned short endY, unsigned char color);
void drawToBuffer(unsigned int x, unsigned int y, unsigned char color);
void renderBuffer(void);

#endif // GRAPHICS_H