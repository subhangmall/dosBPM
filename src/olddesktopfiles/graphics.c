#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <i86.h>
#include <stdlib.h>
#include <malloc.h>
#include "font.h"

unsigned char __huge *vBuffer;

void setPalette(unsigned char colorPalette[16][3]);
void draw(unsigned short x, unsigned short y, unsigned char color);

bool checkVGACompatibility() {
    uint16_t axAfter;

    __asm {
        mov ax, 0x4F00
        int 0x10
        mov axAfter, ax
    }

    return (axAfter == 0x004F);
}


void initVideoMode() {
    unsigned char pal[16][3] = {
    {0x00, 0x00, 0x00}, // 0: Black
    {0x00, 0x00, 0x20}, // 1: Dark Blue (Windows Title Bar)
    {0x00, 0x20, 0x00}, // 2: Dark Green
    {0x01, 0x20, 0x20}, // 3: Windows Teal (The Desktop Background)
    {0x20, 0x00, 0x00}, // 4: Dark Red
    {0x20, 0x00, 0x20}, // 5: Dark Magenta
    {0x20, 0x20, 0x00}, // 6: Mustard/Brown
    {0x30, 0x30, 0x30}, // 7: Light Gray (The main Window/Button color)
    {0x20, 0x20, 0x20}, // 8: Dark Gray (Button Shadows)
    {0x00, 0x00, 0x3F}, // 9: Bright Blue
    {0x00, 0x3F, 0x00}, // 10: Bright Green
    {0x00, 0x3F, 0x3F}, // 11: Bright Cyan
    {0x3F, 0x00, 0x00}, // 12: Bright Red
    {0x3F, 0x00, 0x3F}, // 13: Bright Magenta
    {0x3F, 0x3F, 0x00}, // 14: Bright Yellow
    {0x3F, 0x3F, 0x3F}  // 15: Pure White (Button Highlights)
};

    __asm {
        mov ah, 0x00 // Set video mode
        mov al, 0x12 // 640x200 pixel graphics, 16 colors
        int 0x10 // Call BIOS video interrupt
    }

    
    setPalette(pal);

    vBuffer = (unsigned char __huge *)halloc(307200L, 1); // 640*480 = 307.2 kB
}

void write(unsigned char message[], unsigned char color, int x, int y) {
    uint8_t lineByte;
    unsigned char pixel;
    int j,i,p;
    
    if (color > 15) {
        return; // Invalid color
    }

    for (p = 0; message[p] != '\0'; p++) {
        for (j = 0; j < 8; j++) {
            lineByte = console_font_8x8[message[p] * 8 + j];
            for (i = 0; i < 8; i++) {
                pixel = (lineByte >> (7 - i)) & 0x01;
                if (pixel) {
                    draw(x + i + p*8, y + j, color);
                }
            }
        }
    }
}

void writeWithBackground(unsigned char message[], unsigned char fgColor, unsigned char bgColor, int x, int y) {
    uint8_t lineByte;
    unsigned char pixel, pixelToDraw;
    int j,i,p;
    
    if (fgColor > 15 || bgColor > 15) {
        return; // Invalid color
    }

    for (p = 0; message[p] != '\0'; p++) {
        for (j = 0; j < 8; j++) {
            lineByte = console_font_8x8[message[p] * 8 + j];
            for (i = 0; i < 8; i++) {
                pixel = (lineByte >> (7 - i)) & 0x01;
                if (pixel) {
                    pixelToDraw = fgColor;
                } else {
                    pixelToDraw = bgColor;
                }

                draw(x + i + p*8, y + j, pixelToDraw);
            }
        }
    }
}


// REVIEW MEMORY STRUCTURE LATER
void draw(unsigned short x, unsigned short y, unsigned char color) {
    drawToBuffer(x, y, color);
}

void drawBackground() {
    unsigned short x, y;
    unsigned char black= 0;
    unsigned char white= 15;
    
    for (x = 0; x < 640; x++) {
        for (y = 0; y < 480; y++) {
            draw(x, y, 3); // teal bg
        }
    }
}

void drawToBuffer(unsigned int x, unsigned int y, unsigned char color) {
    unsigned long offset = (640UL * y) + (unsigned long)x;
    vBuffer[offset] = color & 0x0F; // to be safe, mask to 4 bits
}

void renderBuffer(void) {
    unsigned int plane;
    unsigned long startingByteIdx;
    unsigned int byteIdx;
    unsigned char bitMask;
    unsigned char byte;
    volatile unsigned char __far *vga = (volatile unsigned char __far *)MK_FP(0xA000, 0);
    unsigned int vgaIdx;
    unsigned char __huge *p;

    // enable write mode 0
    __asm {
        mov dx, 0x3CE
        mov al, 0x05
        out dx, al
        inc dx
        mov al, 0x00
        out dx, al
    }

    // set bit mask to 0xFF, meaning all pixels will be written
    __asm {
        mov dx, 0x3CE
        mov al, 0x08
        out dx, al
        inc dx
        mov al, 0xFF
        out dx, al
    }

    for (plane = 0; plane < 4; plane++) {
        // select plane to write to
        bitMask = 1 << plane;
        vgaIdx = 0;

        // set map mask to current plane
        __asm {
            mov dx, 0x3C4
            mov al, 0x02
            out dx, al
            inc dx
            mov al, bitMask
            out dx, al
        }

        for(startingByteIdx = 0; startingByteIdx < 307200; startingByteIdx += 8) {
            byte = 0;
            
            // start at this specific byte index
            p = &vBuffer[startingByteIdx];


            // do it manually b/c its faster than looping and having the cpu having to calculate everything
            if (p[0] & bitMask) byte |= 0x80;
            if (p[1] & bitMask) byte |= 0x40;
            if (p[2] & bitMask) byte |= 0x20;
            if (p[3] & bitMask) byte |= 0x10;
            if (p[4] & bitMask) byte |= 0x08;
            if (p[5] & bitMask) byte |= 0x04;
            if (p[6] & bitMask) byte |= 0x02;
            if (p[7] & bitMask) byte |= 0x01;

            vga[vgaIdx++] = byte;
        }
    }

    // reset map mask to 0xFF
    __asm {
        mov dx, 0x3C4
        mov al, 0x02
        out dx, al
        inc dx
        mov al, 0xFF
        out dx, al
    }
}

void drawRectangle(unsigned short x, unsigned short y, unsigned short width, unsigned short height, unsigned char color) {
    unsigned short i, j;
    
    for (i = 0; i < width; i++) {
        for (j = 0; j < height; j++) {
            draw(x + i, y + j, color);
        }
    }
}

void drawLine(unsigned short startX, unsigned short startY, unsigned short endX, unsigned short endY, unsigned char color) {
    // edge case if start and end are the same
    int dx, dy, steps, i;
    float xIncrement, yIncrement, x, y;
    
    if (startX == endX && startY == endY) {
        draw(startX, startY, color);
        return;
    }

    // draw actual line
    dx = (int) endX - (int) startX;
    dy = (int) endY - (int) startY;
    steps = (abs(dx) > abs(dy)) ? abs(dx) : abs(dy);
    xIncrement = dx / (float) steps;
    yIncrement = dy / (float) steps;
    x = startX;
    y = startY;

    for (i = 0; i <= steps; i++) {
        draw((unsigned short)(x + 0.5), (unsigned short)(y + 0.5), color);
        x += xIncrement;
        y += yIncrement;
    }

}
