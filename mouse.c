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

