section .text

; global int0
; global int1
; global int2
; global int3
; global int4
; global int5
; global int6
; global int7
; global int8
; global int9
; global intA
; global intB
; global intC
; global intD
; global intE ; SKIP F BECAUSE 0F IS RESERVED
; global int10
; global int11
; global int12
; global int13
; global int14
; global int15

extern dispatcher

%macro INT_NOERR 1
global int%1
int%1:
    cli
    push 0
    push %1
    jmp dispatcherHandler
%endmacro

%macro INT_ERR 1
global int%1
int%1:
    cli
    push %1
    jmp dispatcherHandler
%endmacro

%assign i 0
%rep 256
    %if i = 15
        ; cause 0x0F is reservedd by Intel, should not touch
    %elif i = 8 || i = 10 || i = 11 || i = 12 || i = 13 || i = 14 || i = 17 || i = 21
        INT_ERR i
    %else
        INT_NOERR i
    %endif
%assign i i+1
%endrep
; NON ERROR CODE INTERRUPTS

; int0: ; divide error
;     cli
;     push 0
;     push 0x0
;     jmp dispatcherHandler

; int1: ; debug exception
;     cli
;     push 0
;     push 0x1 
;     jmp dispatcherHandler

; int2: ; Nonmaskable external interrupt
;     cli
;     push 0
;     push 0x2
;     jmp dispatcherHandler

; int3: ; breakpoint
;     cli
;     push 0
;     push 0x3
;     jmp dispatcherHandler

; int4: ; overflow
;     cli
;     push 0
;     push 0x4
;     jmp dispatcherHandler

; int5: ; BOUND range exceeded
;     cli
;     push 0
;     push 0x5
;     jmp dispatcherHandler

; int6: ; invalid opcode
;     cli
;     push 0
;     push 0x6
;     jmp dispatcherHandler

; int7: ; no math coprocessor
;     cli
;     push 0
;     push 0x7
;     jmp dispatcherHandler

; int9: ; coprocessor segment overrun
;     cli
;     push 0
;     push 0x9
;     jmp dispatcherHandler

; ; NOTE : SKIP INTEL 0F because not allowed to be used

; int10: ; x87 FPU error
;     cli
;     push 0
;     push 0x10
;     jmp dispatcherHandler

; int12: ; machine check; model dependent
;     cli
;     push 0
;     push 0x12
;     jmp dispatcherHandler

; int13: ; machine check; model dependent
;     cli
;     push 0
;     push 0x13
;     jmp dispatcherHandler

; int14: ; virtualization exception
;     cli
;     push 0
;     push 0x14
;     jmp dispatcherHandler




; ; ERROR CODE EXCEPTIONS

; int8: ; double fault
;     cli
;     push 0x08
;     jmp dispatcherHandler

; intA: ; invalid TSS
;     cli
;     push 0x0A
;     jmp dispatcherHandler

; intB: ; segment not present
;     cli
;     push 0x0B
;     jmp dispatcherHandler

; intC: ; stack segment fault
;     cli
;     push 0x0C
;     jmp dispatcherHandler

; intD: ;general protection fault
;     cli
;     push 0x0D
;     jmp dispatcherHandler

; intE: ; page fault
;     cli
;     push 0x0E
;     jmp dispatcherHandler

; int11: ; alignment check
;     cli
;     push 0x11
;     jmp dispatcherHandler

; int15: ; control protection exception
;     cli
;     push 0x15
;     jmp dispatcherHandler

dispatcherHandler:
    pusha
    push ds
    push es
    push fs
    push gs

    mov ax, 0x10
    mov ds, ax
    mov es, ax

    push esp
    call dispatcher
    add esp, 4

    pop gs
    pop fs
    pop es
    pop ds
    popa
    add esp, 8
    ; sti
    iretd