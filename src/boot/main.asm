; NOTE: credit to https://github.com/Cotezzo/asm-protected-mode/ and the OS Dev Wiki for the basic outline, but this file is very heavily modified
; .COM BASED BOOTLOADER

; ==== MEMORY AND ARCH DIRECTIVES ============================================================================ ;
org 0x100
bits 16

; ==== BOOTLOADER MAIN ======================================================================================= ;
main:
    [bits 16]
    mov dx, init_msg
    call print16bit


    ; Setup segment registers and stack pointer
    mov ax, cs
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0xFFFE

    call do_e820

    ; LOAD KERNEL INTO MEMORY

    ; shrink program memory block to get rest free space
    mov bx, 4096 ; shrink to 64kb (WAY more then enough)
    mov ah, 0x4A
    int 0x21
    jc shrink_fail

    ; open file and get handle name
    mov ah, 0x3D
    mov al, 0x02 ; read/write
    ; ds is alreayd set correctly
    mov dx, kernel_to_load_name
    int 0x21
    jc open_file_fail
    mov [kernel_file_handle], ax

    ;;;; get file length

    ; start of file
    mov ah, 0x42
    mov al, 0x01
    mov bx, [kernel_file_handle]
    xor cx, cx
    xor dx, dx
    int 0x21
    jc error_determining_length
    mov [start_file_lo], ax
    mov [start_file_hi], dx
    

    ; end of file
    mov ah, 0x42
    mov al, 0x02
    mov bx, [kernel_file_handle]
    mov cx, 0
    mov dx, 0
    int 0x21
    jc error_determining_length
    mov [end_file_hi], dx
    mov [end_file_lo], ax

    ; reset to start
    mov ah, 0x42
    mov al, 0x00
    mov bx, [kernel_file_handle]
    xor cx, cx
    xor dx, dx
    int 0x21  
    jc failed_to_reset_pos

    ; calculate length
    mov ax, [end_file_hi]
    shl eax, 16
    add ax, [end_file_lo]
    mov bx, [start_file_hi]
    shl ebx, 16
    add bx, [start_file_lo]
    sub eax, ebx
    mov [kernel_file_size], eax

    ; allocate mmeory for kernel to load in
    mov eax, [kernel_file_size]
    shr eax, 4 ; B/C MEMORY ALLOCATIN IS IN PARAGRAPHS
    mov bx, ax
    inc bx ; just to be safe, allocate an extra paragraph
    mov ah, 0x48
    int 0x21
    jc alloc_mem_fail
    mov [seg_to_load_to], ax

    ; setup loaded_krnl_32_pmode_addr
    xor eax, eax
    mov ax, [seg_to_load_to]
    shl eax, 4
    ; offset is 0, so no need to add it
    mov [loaded_krnl_32_pmode_addr], eax

    ; load file
    mov ah, 0x3F
    mov bx, [kernel_file_handle]
    mov ecx, [kernel_file_size]
    and ecx, 0x0000FFFF ; just to be safe
    push ds
    mov ds, [seg_to_load_to]
    mov dx, 0 ; offset is always 0 
    int 0x21
    pop ds
    jc load_file_fail

    ; ==== SWITCH TO 32BIT PROTECTED MODE ==================================================================== ;
    cli

    ; store if the A20 Line is enabled in the E flag
    call a20_check
    cmp ax, 1

    ; if A20 is disabled, enable it and check again
    je .a20_enabled
    call a20_enable
    call a20_check
    cmp ax, 1
    je .a20_enabled

    ; print a20 failure
    ; mov si, text16r_a20_disabled
    ; call print16r
    jmp a20_enable_fail

    .a20_enabled:
    ; mov si, text16r_a20_enabled
    ; call print16r

    ; set right base address for gdt descriptor
    xor eax, eax
    mov ax, ds                  
    shl eax, 4                  
    add eax, gdt                
    mov [gdt_descriptor + 2], eax

    ; load gdt
    lgdt [gdt_descriptor]

    ; GET 32 BIT PMODE ADDRESS FOR FAR JUMP POINTER
    xor eax, eax
    mov ax, cs
    shl eax, 4
    add eax, .32pm
    mov [far_jmp_ptr], eax

    ; switch to pmode
    mov eax, cr0
    or al, 1
    mov cr0, eax

    jmp far dword [far_jmp_ptr]

    ; ==== 32BIT PROTECTED MODE ================== ;
    .32pm: 
    [bits 32]
    xor edx, edx
    mov dx, ds ; DX IS NOW DS

    ; Setup data segments for 32-bit flat memory
    mov ax, 0x10       ; GDT data segment
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov esp, 0x3F0000   

    ; get right address to load address from
    xor eax, eax
    mov eax, edx ; data segment
    shl eax, 4
    add eax, loaded_krnl_32_pmode_addr ; OFFSET OF THIS

    ; load address into ebx
    mov esi, dword [eax]

    ; get size of kernel file
    xor eax, eax
    mov ax, dx
    shl eax, 4
    add eax, kernel_file_size

    ; copy kernel to higher address
    mov edi, 0x100000
    mov ecx, [eax] ; amount of bytes to move is the same as the file size
    inc ecx ; just to be safe
    cld
    rep movsb

    ; PUSH 32 BIT PMODE ADDRESS FOR E820 FREE SPACE POINTER
    xor eax, eax
    mov ax, dx ; DATA SEGMENT
    shl eax, 4
    add eax, e820_free_space
    push eax ; FIRST E820 FREE SPACE ADDRESS

    ; PUSH 32 BIT PMODE ADDRESS FOR E820 FREE SPACE LEN POINTER
    xor eax, eax
    mov ax, dx ; DATA SEGMENT
    shl eax, 4
    add eax, e820_entry_amt
    push eax ; SECOND 32 BIT PMODE FREE SPACE LEN ADDRESS

    call 0x08:0x00100000    

    ; ==== LOOP FOREVER ===================================================================== ;
    .loop:
        hlt
        jmp .loop

%define VGA_TEXT_BUFFER 0xB8000
%define PS2_PORT_DATA 0x60
%define PS2_PORT_COMMAND 0x64

; The keyboard controller handles specific commands
%define PS2_COMMAND_DISABLE_KEYBOARD 0xAD
%define PS2_COMMAND_ENABLE_KEYBOARD 0xAE
%define PS2_COMMAND_READ_FROM_OUTPUT 0xD0
%define PS2_COMMAND_WRITE_TO_OUTPUT 0xD1

print16bit:
    [bits 16]
    mov ah, 0x09
    int 0x21
    ret

;* Checks if the A20 Line is enabled.
;* Old BIOSes might not enable it by default.
;* Output:
;* - AX, 1 if the A20 line is enabled, 0 otherwise.
a20_check:
    [bits 16]
    ; In the bootloader, bytes 511 - 512 are 0xAA55:
    ; comparing [0:7DFE] to this value
    ; should always assert to true.
    ;> cmp byte [es:di], 0x55
    ;> cmp byte [es:di+1], 0xAA
    ; To check whether the A20 line wraps around or
    ; not, try to access the same address + 1MB and
    ; check the value against the one at [0:7DFE].
    ; If the address wraps around and the same
    ; values are accessed, the A20 line is disabled.

    ; Save used registers
    push es
    push di
    push ds
    push si
    
    ; Setup segments and index regs for mem access
    xor ax, ax
    mov es, ax                                      ; Set ES to 0x0000
    mov di, 0x7DFE                                  ; ES:DI = 0000:7DFE --> REAL ADDR: 0x007DFE

    not ax
    mov ds, ax                                      ; Set DS to 0xFFFF
    mov si, 0x7E0E                                  ; DS:SI = FFFF:7E0E --> REAL ADDR: 0x107DFE (ES:DI + 1MB)
                                                    ;  1    0    7    D    F    E
                                                    ; 0001 0000 0111 1101 1111 1110
                                                    ;    '-- 20th bit - if A20 line is disabled, this becomes 0

    ; Set return value to 1 by default
    mov ax, 1

    ; Lazy and insecure implementation:
    ; Read magic number from DS:SI
    ; If the comparison fails, exit
    ; ES:DI -----> 55
    ; ES:DI +1 --> AA
    ; DS:SI -----> 00   (If A20 already enabled)
    ; DS:SI +1 --> 00   (If A20 already enabled)
    ;> cmp byte [ds:si], 0x55
    ;> jne .exit ; Byte NOT equal, mem does NOT wrap

    ; Safe implementation:
    ; Clear previous ES:DI value with v1;
    ; Write v2 to DS:SI (different from v1);
    ; Compare ES:DI against v2.
    ; If memory wraps, modifying DS:SI affects
    ; ES:DI and the value is the same as v2.
    ; If not, A20 is already enabled (ES:DI value
    ; is still v1).
    ; The values previously stored at ES:DI and
    ; DS:SI are restored to their previous state.
    push word [es:di]
    push word [ds:si]
    mov byte [es:di], 0x00  ; Set ES:DI to v1
    mov byte [ds:si], 0xFF  ; Set DS:SI to v2
    cmp byte [es:di], 0xFF  ; Check ES:DI for v2
    pop word [ds:si]
    pop word [es:di]
    jne .exit   ; Byte NOT equal, mem does NOT wrap

    ; If no exit is called, all cmps set the E flag:
    ; A20 disabled, memory wraps around; ret 0
    xor ax, ax

    ; If exit is called, a cmp failed:
    ; A20 enabled, memory doesn't wrap around; ret 1
    ; 1 is the default value of ax, jumping here
    ; avoids the xor that sets ax to 1
    .exit:

    ; Restore used registers and return
    pop si
    pop ds
    pop di
    pop es
    ret

;* Uses the hardware I/O ports on the Keyboard 
;* Controller to try enabling the A20 Gate.
a20_enable:
    [bits 16]
    push ax

    ;! Before writing command to input buffer,
    ;! always wait for it to be empty first.

    ; Disable keyboard to avoid interruptions.
    call ps2_wait_ibuf_empty                        ; Wait for the input buffer to be empty
    mov al, PS2_COMMAND_DISABLE_KEYBOARD
    out PS2_PORT_COMMAND, al

    ; Send read command to get controller configuration
    ; byte.
    ; Wait until the config byte can be read from
    ; the output buffer, read it from data port
    ; and save to AL register.
    call ps2_wait_ibuf_empty                        ; Wait for the input buffer to be empty
    mov al, PS2_COMMAND_READ_FROM_OUTPUT
    out PS2_PORT_COMMAND, al
    call ps2_wait_obuf_full                         ; Wait for the output buffer to be full
    in al, PS2_PORT_DATA
    push ax                                         ; Save AL for later, it is used for I/O operations

    ; Send write command so that we can change
    ; the value of the config byte.
    call ps2_wait_ibuf_empty                        ; Wait for the input buffer to be empty
    mov al, PS2_COMMAND_WRITE_TO_OUTPUT
    out PS2_PORT_COMMAND, al

    ; Second bit of config byte toggles the A20
    ; line, set it and write new byte to DATA port.
    call ps2_wait_ibuf_empty                        ; Wait for the input buffer to be empty
    pop ax                                          ;! pop AFTER waiting, since wait does not restore AL value
    or al, 2
    out PS2_PORT_DATA, al

    ; Re-enable keyboard
    call ps2_wait_ibuf_empty                        ; Wait for the input buffer to be empty
    mov al, PS2_COMMAND_ENABLE_KEYBOARD
    out PS2_PORT_COMMAND, al

    ; Wait for process to end and restore registers
    call ps2_wait_ibuf_empty
    pop ax
    ret

;* Waits until the PS/2 input buffer can be used.
;! Uses AX register without restoring its state.
; The controller is slow, loop until the previous
; process finished and the input buffer is free.
ps2_wait_ibuf_empty:
    [bits 16]

    ; Command port returns the status register, it
    ; shows the controller's state with flags
    in al, PS2_PORT_COMMAND

    ; Second status register bit: input buffer state
    ; 0: the buffer is empty; 1: the buffer is full
    test al, 2  ; al & 2, discard result, set flags

    ; If the buffer is full, keep waiting
    jnz ps2_wait_ibuf_empty

    ; If the buffer is empty, return
    ret

;* Waits until the PS/2 output buffer is full.
;! Uses AX register without restoring its state.
; The controller is slow, loop until the previous
; process finished and the output buffer is filled.
ps2_wait_obuf_full:
    [bits 16]

    ; Command port returns the status register, it
    ; shows the controller's state with flags
    in al, PS2_PORT_COMMAND

    ; First status register bit: output buffer state
    ; 0: the buffer is empty; 1: the buffer is full
    test al, 1

    ; If the buffer is empty, keep waiting
    jz ps2_wait_obuf_full

    ; If the buffer is full, return
    ret

exit_to_dos:
    [bits 16]
    mov ah, 0x4C
    int 0x21

open_file_fail:
    mov dx, open_file_fail_msg
    call print16bit
    jmp exit_to_dos

shrink_fail:
    mov dx, shrink_fail_msg
    call print16bit
    jmp exit_to_dos

alloc_mem_fail:
    mov dx, alloc_mem_fail_msg
    call print16bit
    jmp exit_to_dos

error_determining_length:
    mov dx, error_determining_length_msg
    call print16bit
    jmp exit_to_dos

load_file_fail:
    mov dx, load_file_fail_msg
    call print16bit
    jmp exit_to_dos

a20_enable_fail:
    mov dx, a20_enable_fail_msg
    call print16bit
    jmp exit_to_dos

failed_to_reset_pos:
    mov dx, failed_to_reset_pos_msg
    call print16bit
    jmp exit_to_dos

mmap_ent equ e820_entry_amt            

do_e820:
    mov di, e820_free_space
	xor ebx, ebx		; ebx = 0
	xor bp, bp		; entry count in bp
	mov edx, 0x0534D4150	; "SMAP"
	mov eax, 0xe820
	mov [es:di + 20], dword 1	; force a valid ACPI 3.X entry
	mov ecx, 24
	int 0x15
	jc short .failed	; carry on first call -> unsupported function
	mov edx, 0x0534D4150	
	cmp eax, edx		; success = eax must have been set to "SMAP"
	jne short .failed
	test ebx, ebx		; ebx = 0 -> fail
	je short .failed
	jmp short .jmpin
.e820lp:
	mov eax, 0xe820		; eax, ecx get trashed on every int 0x15 call
	mov [es:di + 20], dword 1	; force a valid ACPI 3.X entry
	mov ecx, 24		; ask for 24 bytes again
	int 0x15
	jc short .e820f		; carry set means "end of list already reached"
	mov edx, 0x0534D4150	; repair potentially trashed register
.jmpin:
	jcxz .skipent		; skip 0 length entries
	cmp cl, 20		; got a 24 byte ACPI 3.X response?
	jbe short .notext ; jmp if below or equl, 20 byte response
	test byte [es:di + 20], 1	; if so: is the "ignore this data" bit clear?
	je short .skipent
.notext:
	mov ecx, [es:di + 8]	; get lower uint32_t of memory region length
	or ecx, [es:di + 12]	; "or" it with upper uint32_t to test for zero
	jz .skipent		; if length uint64_t is 0, skip entry
	inc bp			; got a good entry: ++count, move to next storage spot
	add di, 24
.skipent:
	test ebx, ebx		; if ebx resets to 0, list is complete
	jne short .e820lp
.e820f:
	mov [es:mmap_ent], bp	; store the entry count
	clc			
	ret
.failed:
    ; mov ah, 0x02
    ; mov dl, 'B'
    ; int 0x21

	stc			
	ret


gdt:
    ; 1st entry: must be empty
    dq 0 ; NULL DESC

    ; 2nd entry: 32b code segment, flat memory model
.gdt_selector_32pm_cs:
    dw 0xFFFF                                       ; Limit (0-15 bits)
    dw 0                                            ; Base (0-15 bits)
    db 0                                            ; Base (16-23 bits)
    db 1001101_0b                              ; Access Byte (reversed in little endian), left to right:
                                                    ; Present Bit, always 1 for a valid segment
                                                    ; Privilege (2b): privilige level required (Ring 0, 1, 2, 3)
                                                    ; Type: 0 for system segment, 1 for code or data segment
                                                    ; Executable: 0 for data segment, 1 for code segment
                                                    ; Direction/Confirming (code selectors):
                                                    ;   0: execution only allowed for specified ring privilege
                                                    ;   1: execution only allowed for equal/lower ring level,
                                                    ;      privilege remains but can read and execute the code
                                                    ;   (data selectors):
                                                    ;   0: Segments grows (ex: stack) up
                                                    ;   1: Segments grows down (offset > limit)
                                                    ; RW (code segments): 1 enables read (write never allowed)
                                                    ;    (data segments): 1 enables write (read always allowed)
                                                    ; Accessed: set by CPU when segment is accessed
    db 11001111b                                ; Flags (reversed in little endian), left to right:
                                                    ; Granularity: 1 to shl the limit value (with 1s, not 0s)
                                                    ; Size: 0 for 16b protected mode, 1 for 32b protected mode
                                                    ; Long-Mode: 1 for 64b protected mode (Size must be 0)
                                                    ; Limit (16-19 bits)
    db 0                                            ; Base (24-31 bits)

    ; 3rd entry: 32b data segment, flat memory model
.gdt_selector_32pm_ds:
    dw 0xFFFF
    dw 0
    db 0
    db 10010010b                              ; access byte
    db 11001111b
    db 0

; In order to load GDT, another structure is
; requried: the GDT descriptor.
; It contains GDT's size - 1 (2B) and address (4B).
gdt_descriptor:
    dw gdt_descriptor - gdt - 1             ; Calculate size using address offsets
    dd gdt

; a20_success: db "A20DONE$"
init_msg: db "Booting up the kernel$"
kernel_to_load_name: db "KERNEL.BIN", 0
kernel_file_handle: dw 0
open_file_fail_msg: db "Failed to open file", '\n', '$'
shrink_fail_msg: db "Failed to shrink BOOT.COM's memory usage", '\n', '$'
alloc_mem_fail_msg: db "Failed to allocate sufficient memory for KERNEL.BIN", '\n', '$'
error_determining_length_msg: db "Failed to determine length of KERNEL.BIN", '\n', '$'
load_file_fail_msg: db "Failed to load KERNEL.BIN into memory", '\n', '$'
a20_enable_fail_msg: db "Failed to enable A20", '\n', '$'
failed_to_reset_pos_msg: db "Failed to reset file seek position to start", '\n', '$'

e820_free_space: times 4096 db 0
e820_entry_amt: dw 0

seg_to_load_to: dd 0
loaded_krnl_32_pmode_addr: dd 0
end_file_lo: dw 0
end_file_hi: dw 0
start_file_hi: dw 0
start_file_lo: dw 0
kernel_file_size: dd 0

align 4
far_jmp_ptr:
    .offset:   dd 0            ; 4 bytes (Gets overwritten at runtime with EAX)
    .selector: dw 0x08         ; 2 bytes (Hardcoded flat code segment selector)