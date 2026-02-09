[bits 32]           
section .text    
global pModeMain     
extern kentry          

pModeMain:
    mov ax, 0x10        ; data segment selector
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax

    mov esp, 0x90000    ; setup stack
    call kentry          ; jump to C

hang:
    hlt
    jmp hang