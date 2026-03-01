section .text

global int0
global int1
global int2
global int3
global int4
global int5
global int6
global int7
global int8
global int9
global intA
global intB
global intC
global intD
global intE ; SKIP F BECAUSE 0F IS RESERVED
global int10
global int11
global int12
global int13
global int14
global int15

extern commonHandler

; NON ERROR CODE INTERRUPTS

int0: ; divide error
    cli
    push 0
    push 0x0
    jmp isr_common

int1: ; debug exception
    cli
    push 0
    push 0x1 
    jmp isr_common

int2: ; Nonmaskable external interrupt
    cli
    push 0
    push 0x2
    jmp isr_common

int3: ; breakpoint
    cli
    push 0
    push 0x3
    jmp isr_common

int4: ; overflow
    cli
    push 0
    push 0x4
    jmp isr_common

int5: ; BOUND range exceeded
    cli
    push 0
    push 0x5
    jmp isr_common

int6: ; invalid opcode
    cli
    push 0
    push 0x6
    jmp isr_common

int7: ; no math coprocessor
    cli
    push 0
    push 0x7
    jmp isr_common

int9: ; coprocessor segment overrun
    cli
    push 0
    push 0x9
    jmp isr_common

; NOTE : SKIP INTEL 0F because not allowed to be used

int10: ; x87 FPU error
    cli
    push 0
    push 0x10
    jmp isr_common

int12: ; machine check; model dependent
    cli
    push 0
    push 0x12
    jmp isr_common

int13: ; machine check; model dependent
    cli
    push 0
    push 0x13
    jmp isr_common

int14: ; virtualization exception
    cli
    push 0
    push 0x14
    jmp isr_common




; ERROR CODE EXCEPTIONS

int8: ; double fault
    cli
    push 0x08
    jmp isr_common

intA: ; invalid TSS
    cli
    push 0x0A
    jmp isr_common

intB: ; segment not present
    cli
    push 0x0B
    jmp isr_common

intC: ; stack segment fault
    cli
    push 0x0C
    jmp isr_common

intD: ;general protection fault
    cli
    push 0x0D
    jmp isr_common

intE: ; page fault
    cli
    push 0x0E
    jmp isr_common

int11: ; alignment check
    cli
    push 0x11
    jmp isr_common

int15: ; control protection exception
    cli
    push 0x15
    jmp isr_common

isr_common:
    pusha
    push ds
    push es
    push fs
    push gs

    mov ax, 0x10
    mov ds, ax
    mov es, ax

    push esp
    call commonHandler
    add esp, 4

    pop gs
    pop fs
    pop es
    pop ds
    popa
    add esp, 8
    ; sti
    iretd

isr_err:
    iretd