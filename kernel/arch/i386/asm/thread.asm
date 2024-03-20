bits 32
section .text

extern thread_run_entry
extern thread_run_param
extern thread_run_esp
extern thread_run_ebp

global thread_run

thread_run:
    ; setup the thread stack
    mov esp, [thread_run_esp]
    mov ebp, [thread_run_ebp]
    
    ; pushing zeros to mark an end for call traces
    mov eax, 0
    push eax
    push eax

    ; prepare function calling
    mov eax, [thread_run_param]
    push eax
    mov eax, [thread_run_entry]

    ; enabling interrupts
    sti

    ; call the thread entry function with param
    call eax
    add esp, 4

    ; exit thread
    int 0xc2

