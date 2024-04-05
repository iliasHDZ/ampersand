bits 32

section .text
global _start

extern _init_libc

extern _init
extern _fini

extern main
extern exit

extern __main_argc
extern __main_argv

_start:
    mov ebp, 0
    push ebp
    push ebp
    mov ebp, esp

    call _init_libc

    call _init

    mov ebx, [__main_argv]
    mov eax, [__main_argc]
    push ebx
    push eax
    call main
    add esp, 8
    push eax

    call _fini

    call exit
    jmp $