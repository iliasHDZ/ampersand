bits 32

section .text
global _start

extern _init_libc

extern _init
extern _fini

extern main
extern exit

_start:
    mov ebp, 0
    push ebp
    push ebp
    mov ebp, esp

    call _init_libc

    call _init

    call main
    push eax

    call _fini

    call exit
    jmp $