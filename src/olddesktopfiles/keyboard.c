
#include <stdbool.h>
#include <i86.h>

#define KEYBOARD_BUFFER_SIZE (unsigned char) 32
#define KEYBOARD_MAP_SIZE 256

volatile unsigned char keyboard_buffer[KEYBOARD_BUFFER_SIZE];
volatile unsigned char keyboard_map[KEYBOARD_MAP_SIZE];
volatile unsigned char keyIdx = 0;

unsigned volatile char scrollNumberCapsLockStatusByte;

unsigned volatile int oldOffset;
unsigned volatile int oldSegment;
unsigned volatile int curDS;

void hookInterrupt();
void disableKeyboardSet2to1Translation();

void keyboardDriver() {
    
    // enable keyboard scanning
    __asm {
        mov al, 0xF4
        mov dx, 0x64
        out dx, al
    }

    disableKeyboardSet2to1Translation();
    hookInterrupt();
}

void disableKeyboardSet2to1Translation() {
    unsigned char status, config;

    // wait till keyboard ready to read
    do {
        __asm {
            mov dx, 0x64
            in al, dx
            mov status, al
        } 
    } while (status & 0x02);

    // read current config byte; start by requesting it
    __asm {
        mov al, 0x20
        mov dx, 0x64
        out dx, al
    }

    // wait till keyboard has data ready
    do {
        __asm {
            mov dx, 0x64
            in al, dx
            mov status, al
        } 
    } while (!(status & 0x01));

    // actually read the byte once ready
    __asm {
        mov dx, 0x60
        in al, dx
        mov config, al
    }

    config &= 0b10111111;

    // wait till keyboard ready to read
    do {
        __asm {
            mov dx, 0x64
            in al, dx
            mov status, al
        } 
    } while (status & 0x02);

    // write command byte
    __asm {
        mov dx, 0x64
        mov al, 0x60
        out dx, al
    }

    // wait for keyboard to be ready to read
    do {
        __asm {
            mov dx, 0x64
            in al, dx
            mov status, al
        } 
    } while (status & 0x02);

    // write new config byte
    __asm {
        mov al, config
        mov dx, 0x60
        out dx, al
    } 
}

void updateKeyboardStatus(unsigned char scancode) {
    if (keyIdx >= KEYBOARD_BUFFER_SIZE) return;

    keyboard_buffer[keyIdx] = scancode;
    keyIdx++;
}

void __loadds interrupt far keyboardInterrupt() {
    __asm {
        // cli

        // push ax
        // push bx
        // push ds
        // push dx
        
        // get right data segment
        // mov ax, curDS
        // mov ds, ax

        // read keyboard input
        mov dx, 0x60
        in al, dx
        push ax

        // update keyboardMap
        mov al, keyIdx
        mov bl, KEYBOARD_BUFFER_SIZE
        inc al
        cmp al, bl
        // if didn't exceed buffer size, then register key
        jae done // don't store
 
        store:
            pop ax
            mov bx, offset keyboard_buffer
            add bx, keyIdx
            mov bx, al
            inc keyIdx
            push ax

        
        
        done: 
            pop ax

            // say interrupt comlete
            mov dx, 0x20
            mov al, 0x20
            out dx, al   
        
            // notify keyboard that read interrupt
            mov dx, 0x61
            in al, dx
            mov ah, al
            or al, 0x80
            out dx, al
            mov al, ah
            out dx, al

            // sti

    }
}

void hookInterrupt() {
    void (far *func_ptr)(void) = keyboardInterrupt;
    unsigned int s = FP_SEG(func_ptr);
    unsigned int o = FP_OFF(func_ptr);
    struct SREGS sregs;
    segread(&sregs);
    curDS = sregs.ds;

    // store old interrupt address
    __asm {
        cli
        mov ah, 0x35
        mov al, 0x09
        int 0x21
        mov oldSegment, es
        mov oldOffset, bx
        sti
    }

    // set new interrupt address
    __asm {
        cli
        push ds
        mov al, 0x09
        mov dx, o
        mov ds, s
        mov ah, 0x25
        int 0x21
        pop ds
        sti
    }
}

void sendLightStatusByte() {
    unsigned char status;

    do {
        __asm {
            mov dx, 0x64
            in al, dx
            mov status, al
        }
    } while (status & 0x02);

    __asm {
        mov dx, 0x60
        mov al, 0xED
        out dx, al
    }

    do {
        __asm {
            mov dx, 0x64
            in al, dx
            mov status, al
        }
    } while (status & 0x02);

    __asm {
        mov dx, 0x60
        mov al, scrollNumberCapsLockStatusByte
        out dx, al
    }
}

void setCapsLockLightStatus(bool val) {
    if (val) {
        scrollNumberCapsLockStatusByte |= 0b00000100;
    } else {
        scrollNumberCapsLockStatusByte &= 0b00000011;
    }
    sendLightStatusByte();
}

void setScrollLockLightStatus(bool val) {
    if (val) {
        scrollNumberCapsLockStatusByte |= 0b00000001;
    } else {
        scrollNumberCapsLockStatusByte &= 0b00000110;
    }
    sendLightStatusByte();
}

void setNumLockLightStatus(bool val) {
    if (val) {
        scrollNumberCapsLockStatusByte |= 0b00000010;
    } else {
        scrollNumberCapsLockStatusByte &= 0b00000101;
    }
    sendLightStatusByte();
}
