#include <stdbool.h>
struct MouseState {
    int x;
    int y;
    bool leftButton;
    bool rightButton;
    bool centerButton;
};

void dispCursor(void) {
    __asm {
        mov ax, 0x01 // display mouse cursor
        int 0x33
    }
}

void hideCursor(void) {
    __asm {
        mov ax, 0x02 // hide mouse cursor
        int 0x33
    }
}

void getMousePos(struct MouseState* m) {
    unsigned int buttonStates;
    int x, y;

    __asm {
        mov ax, 0x03
        int 0x33
        mov x, cx
        mov y, dx
        mov buttonStates, bx
    }

    m->x = x;
    m->y = y;

    m->leftButton = (buttonStates & 1) == 1;
    m->rightButton = (buttonStates & 2) == 2;
    m->centerButton = (buttonStates & 4) == 4;
}



