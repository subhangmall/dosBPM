#include <conio.h>
#include <stdint.h>
#include "mouse.h"
#include "graphics.h"



void main() {
    int dx, dy;
    // uint8_t majorVersion, minorVersion;

    initVideoMode();
    dispCursor();
    drawBackground();
    drawRectangle(100, 100, 200, 150, 12); // Draw a rectangle at (100,100) with width 200, height 150 and color 12

    write("Hello, World!", 10);
    
    // __asm {
    //     mov ah, 0x30
    //     int 0x21
    //     mov majorVersion, al
    //     mov minorVersion, ah
    //     mov dl, majorVersion
    //     add dl, 0x30
    //     mov ah, 0x02
    //     int 0x21
    //     mov dl, minorVersion
    //     add dl, 0x30
    //     mov ah, 0x02
    //     int 0x21
    // }
    
    // checkModes();
    
    
    getch(); // Wait for a key press
    
}