#include <stdint.h>
#include <stdio.h>
#include "mouse.h"
#include "graphics.h"
#include "window.h"




void main() {
    int dx, dy;
    int i, j;
    struct Window progMgr;
    struct MouseState mouse;
    char posStr[50];
    // uint8_t majorVersion, minorVersion;

    initVideoMode();
    dispCursor();
    drawBackground();

    progMgr.x = 100;
    progMgr.y = 50;
    progMgr.width = 400;
    progMgr.height = 300;
    progMgr.priority = 1;
    progMgr.isFocused = true;
    strcpy(progMgr.title, "Program Manager");
    

    renderWindow(&progMgr);
    getMousePos(&mouse);
    write("Mouse Position:", 15, 10, 10);
    sprintf(posStr, "X: %d, Y: %d", mouse.x, mouse.y);
    write(posStr, 15, 10, 25);


    renderBuffer();
    
    
    getch(); // Wait for a key press
    
}