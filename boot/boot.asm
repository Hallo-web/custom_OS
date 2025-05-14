; Multiboot header for GRUB
section .multiboot
align 4
    dd 0x1BADB002         ; magic number
    dd 0x00               ; flags
    dd -(0x1BADB002)      ; checksum

; Set up the stack
section .bss
align 16
stack_bottom:
    resb 16384            ; Reserve 16 KiB for stack
stack_top:

section .text
global _start
extern kernel_main        ; Defined in kernel.c

_start:
    ; Set up the stack
    mov esp, stack_top
    
    ; Call the kernel main function
    call kernel_main
    
    ; If kernel returns, halt the CPU
    cli                   ; Disable interrupts
.hang:
    hlt                   ; Halt the CPU
    jmp .hang             ; Jump to .hang if hlt doesn't work