[org 0x100]
[bits 16]

jmp _start

section .data
    allocMemFailure db 'Failed to allocate memory for initial kernel$'
    allocMemSuccess db 'Allocated memory for initial kernel$'
    openFileFailure db 'Failed to open file INITKERNEL.BIN$'
    openFileSuccess db 'Opened file INITKERNEL.BIN$'
    readFileFailure db 'Failed to read file INITKERNEL.BIN$'
    readFileSuccess db 'Loaded file INITKERNEL.BIN into memory. Now bootstrapping into the initial kernel$'
    fileName db 'INITIALKRNL.BIN', 0
    startSegment dw 0x0000
    handle dw 0x0000

    gdtStart:
        dq 0x0000000000000000 ; null desc
        dq 0x00CF9A000000FFFF   ; code selector = 0x08
        dq 0x00CF92000000FFFF   ; data selector = 0x10
        
    gdtrStart:
        dw 24
        dd gdtLinAddr


section .text
    global _start

_start:
    mov ax, cs
    mov ds, ax

    ; allocate memory, store in ds
    mov ax, 0x4800
    mov bx, 0x0100
    int 0x21
    jc cannotAllocateMemory
    mov [startSegment], ax

    mov ah, 09h
    lea dx, allocMemSuccess
    int 21h

    mov dl, 13  ; Carriage return
    mov ah, 02h
    int 21h

    mov dl, 10  ; Line feed
    mov ah, 02h
    int 21h

    ; open "INITIALKRNL.BIN"
    mov ax, 0X3D00
    mov dx, fileName
    int 21h
    jc cannotOpenFile
    mov [handle], ax

    mov ah, 09h
    lea dx, openFileSuccess
    int 21h

    mov dl, 13  ; Carriage return
    mov ah, 02h
    int 21h

    mov dl, 10  ; Line feed
    mov ah, 02h
    int 21h

    ; read "INITIALKRNL.BIN" at the specified address
    
    mov bx, [handle]
    mov cx, 4096 ; 4096 bytes, the size of the file
    push ds
    mov ax, [startSegment]
    mov ds, ax
    mov ah, 3Fh
    int 21h
    jc cannotReadFile
    pop ds
    
    mov ah, 09h
    lea dx, readFileSuccess
    int 21h

    mov dl, 13  ; Carriage return
    mov ah, 02h
    int 21h

    mov dl, 10  ; Line feed
    mov ah, 02h
    int 21h

    cli

    lgdt [gdtrStart]

    mov eax, cr0
    or eax, 1
    mov cr0, eax

    jmp 0x08:pmodeEnter

cannotAllocateMemory:
    mov ah, 09h
    lea dx, allocMemFailure
    int 21h
    mov ah, 4Ch   ; DOS terminate program
    int 21h

cannotOpenFile:
    mov ah, 0x09
    lea dx, openFileFailure
    int 21h
    mov ah, 4Ch   ; DOS terminate program
    int 21h

cannotReadFile:
    pop ds
    mov ah, 0x09
    lea dx, readFileFailure
    int 21h
    
[bits 32]
pmodeEnter:
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax

    mov esp, 0x90000