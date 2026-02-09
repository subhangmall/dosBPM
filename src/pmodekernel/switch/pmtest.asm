org 0x100
bits 16

start:
    in al, 0x92
    or al, 2
    out 0x92, al

    xor eax, eax
    mov ax, cs
    shl eax, 4
    add eax, gdt_start
    mov [gdt_desc + 2], eax

    cli

    lgdt [gdt_desc]

    mov eax, cr0
    or eax, 1
    mov cr0, eax

    ; far jump flushes prefetch queue
    jmp 0x08:pmode_entry



gdt_start:
    dq 0x0000000000000000      ; null descriptor

    dq 0x00CF9A000000FFFF      ; code segment (0x08)
    dq 0x00CF92000000FFFF      ; data segment (0x10)

gdt_end:

gdt_desc:
    dw gdt_end - gdt_start - 1
    dd 0                       ; filled at runtime


bits 32
pmode_entry:
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax

    mov esp, 0x90000

    ; write '!' to top-left of screen
    mov dword [0xB8000], 0x1F211F21

hang:
    cli
    hlt
    jmp hang
