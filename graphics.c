#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <i86.h>
#include "font.h"

void setPalette(unsigned char colorPalette[16][3]);
void draw(unsigned short x, unsigned short y, unsigned char color);

bool checkSVGACompatibility() {
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
}

void setPalette(unsigned char colorPalette[16][3]) {
    unsigned char colorPaletteIndex, r, g, b;
    for (colorPaletteIndex = 0; colorPaletteIndex < 16; colorPaletteIndex++) {
        r = colorPalette[colorPaletteIndex][0];
        g = colorPalette[colorPaletteIndex][1];
        b = colorPalette[colorPaletteIndex][2];
        
        __asm {
            mov dx, 0x3C8
            mov al, colorPaletteIndex 
            out dx, al // set palette index
            mov dx, 0x3C9
            mov al, r  // Red
            out dx, al
            mov al, g  // Green
            out dx, al
            mov al, b  // Blue
            out dx, al
        }
    }
}

void write(unsigned char message[], unsigned char color) {
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
                    draw(20 + i + p*8, 20 + j, color);
                }
            }
        }
    }
}

void writeWithBackground(unsigned char message[], unsigned char fgColor, unsigned char bgColor) {
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

                draw(20 + i + p*8, 20 + j, pixelToDraw);
            }
        }
    }
}


// REVIEW MEMORY STRUCTURE LATER
void draw(unsigned short x, unsigned short y, unsigned char color) {
    // offset = (horizontal_res/8) * y + (x/8)
    // bit = 7 - (x % 8)
    unsigned int offset = (80 * y) + (x/8);
    unsigned int bit = 7 - (x % 8);
    volatile unsigned char __far *vga = (volatile unsigned char __far *)MK_FP(0xA000, 0);
    unsigned char dummy;
    unsigned char bitMask = 1 << bit;
    
    // address map mask register and enable all bitplanes
    __asm {
        mov dx, 0x3C4
        mov al, 0x02
        out dx, al
        inc dx
        mov al, 0x0F
        out dx, al
    }

    // set write mode to 2
    __asm {
        mov dx, 0x3CE
        mov al, 0x05
        out dx, al
        inc dx
        mov al, 0x02
        out dx, al
    }

    // send bit mask to bit mask register (only edit the specific pixel)
    __asm {
        mov dx, 0x3CE
        mov al, 0x08
        out dx, al
        inc dx
        mov al, bitMask
        out dx, al
    }
    
    dummy = vga[offset]; // dummy read to make hardware copy current data of 4 planes to internal latches
    
    vga[offset] = color; // write color data (only the bitplanes selected by bit mask will be affected)

    // reset 

    // set bit mask register back to 0xFF
    __asm {
        mov dx, 0x3CE
        mov al, 0x08
        out dx, al
        inc dx
        mov al, 0xFF
        out dx, al
    }

    // set write mode back to 0
    __asm {
        mov dx, 0x3CE
        mov al, 0x05
        out dx, al
        inc dx
        mov al, 0x00
        out dx, al
    }
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

void drawRectangle(unsigned short x, unsigned short y, unsigned short width, unsigned short height, unsigned char color) {
    unsigned short i, j;
    
    for (i = 0; i < width; i++) {
        for (j = 0; j < height; j++) {
            draw(x + i, y + j, color);
        }
    }
}
