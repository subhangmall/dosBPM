// similar in spirit to the real initramfs
#include <stdint.h>

// load the kernel into address 0x20000
int loadKrnl() {
    char kernelName[] = "KERNEL.BIN";
    uint16_t fileHandle = 0x0000;
    uint8_t status = 0x00;
    char* kNamePtr = kernelName;

    __asm {
        mov ah, 3Dh
        mov al, 0
        mov dx, kNamePtr
        int 21h
        jc err

        mov fileHandle, ax

        mov ah, 3Fh
        mov bx, fileHandle
        mov cx, 0xFFFF
        xor dx, dx      
        
        push ds
        mov ax, 2000h
        mov ds, ax
        int 21h
        pop ds

        mov ah, 3Eh
        mov bx, fileHandle
        int 21h
        
        jmp done

    err:
        or status, 01h   

    done:
    }

    return !(status & 0x01);
}