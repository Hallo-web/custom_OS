; Multiboot header for GRUB
MBALIGN     equ  1 << 0
MEMINFO     equ  1 << 1
FLAGS       equ  MBALIGN | MEMINFO
MAGIC       equ  0x1BADB002
CHECKSUM    equ -(MAGIC + FLAGS)

section .multiboot
align 4
    dd MAGIC
    dd FLAGS
    dd CHECKSUM

; Reserve a stack for the initial thread
section .bss
align 16
stack_bottom:
    resb 16384 ; 16 KiB
stack_top:

; The kernel entry point
section .text
global _start
extern kernel_main

_start:
    mov esp, stack_top  ; Set up the stack pointer

    ; Call the kernel
    extern kernel_main
    call kernel_main

    ; Enter an infinite loop if the kernel returns
    cli                  ; Disable interrupts
.hang:
    hlt                  ; Halt the CPU
    jmp .hang            ; Just in case