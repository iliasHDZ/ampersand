global _start

extern kernel_begin_phys
extern kernel_end_phys

extern kernel_main
extern _init
extern _fini

extern multiboot_magic_number
extern multiboot_info_structure

extern kernel_entry

MAX_PAGE_TABLES equ 4
PAGE_ENTRY_BITS equ 3

section .boot

align 4096
boot_page:

; ############################
; #### MULTIBOOT 2 HEADER ####
; ############################
header_start:
    dd 0
    dd 0

    dd 0xe85250d6

    dd 0

    dd header_end - header_start

    dd 0x100000000 - (0xe85250d6 + 0 + (header_end - header_start))

    dw 0
    dw 0
    dd 8
header_end:

; ########################
; #### MAIN BOOT CODE ####
; ########################

entry_eax: dd 0
entry_ebx: dd 0

kernel_begin_phy_addr: dd 0
kernel_end_phy_addr: dd 0
multiboot_info_size: dd 0
multiboot_info_dst: dd 0

_start:
    cli

    mov [entry_eax], eax
    mov [entry_ebx], ebx

    lgdt [gdt_descriptor]
    jmp CODE_SEG:gdt_changed

gdt_changed:
    mov ax, DATA_SEG
    mov ds, ax
    mov ss, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    mov eax, kernel_begin_phys
    and eax, 0x0ffff000
    mov [kernel_begin_phy_addr], eax

    mov eax, kernel_end_phys
    and eax, 0x0ffff000
    add eax, 0x00001000
    mov [kernel_end_phy_addr], eax

    mov ebx, [entry_ebx]
    mov eax, [ebx]
    and eax, 0xfffffffc
    add eax, 0x00000004
    mov [multiboot_info_size], eax

    and eax, 0xfffff000
    add eax, 0x00001000
    mov ebx, [kernel_end_phy_addr]
    mov [multiboot_info_dst], ebx
    add [kernel_end_phy_addr], eax

    mov ebx, [entry_ebx]
    mov ecx, 0x1000
    mov edx, ebx
    add edx, [multiboot_info_size]
.mbi_loop1:
    cmp ebx, edx
    je .mbi_loop_end1
    mov eax, [ebx]
    add ebx, 4
    mov [ecx], eax
    add ecx, 4
    jmp .mbi_loop1
.mbi_loop_end1:

    mov ebx, 0x1000
    mov ecx, [multiboot_info_dst]
    mov edx, ebx
    add edx, [multiboot_info_size]
.mbi_loop2:
    cmp ebx, edx
    je .mbi_loop_end2
    mov eax, [ebx]
    add ebx, 4
    mov [ecx], eax
    add ecx, 4
    jmp .mbi_loop2
.mbi_loop_end2:

    mov eax, [multiboot_info_dst]
    sub eax, [kernel_begin_phy_addr]
    add eax, 0xC0000000
    mov [entry_ebx], eax

    mov ebx, page_table
    mov ecx, [kernel_begin_phy_addr]
.table_loop:
    mov eax, [kernel_end_phy_addr]
    cmp ecx, eax
    je  .table_loop_end
    mov eax, ecx
    
    or  eax, PAGE_ENTRY_BITS
    mov [ebx], eax
    
    add ebx, 4
    add ecx, 0x00001000
    jmp .table_loop
.table_loop_end:
    mov eax, page_dir
    mov cr3, eax

    mov eax, cr0
    or eax, 0x80000000
    mov cr0, eax

    mov eax, [entry_eax]
    mov ebx, [entry_ebx]

    mov [multiboot_magic_number], eax
    mov [multiboot_info_structure],  ebx

    jmp _entry

; #################################
; #### GLOBAL DESCRIPTOR TABLE ####
; #################################

gdt_start:
    dd 0x0
    dd 0x0

gdt_code: 
    dw 0xffff
    dw 0x0
    db 0x0
    db 10011010b
    db 11001111b
    db 0x0

gdt_data:
    dw 0xffff
    dw 0x0
    db 0x0
    db 10010010b
    db 11001111b
    db 0x0

gdt_end:

gdt_descriptor:
    dw gdt_end - gdt_start - 1
    dd gdt_start

CODE_SEG equ gdt_code - gdt_start
DATA_SEG equ gdt_data - gdt_start

; #############################
; #### INITIAL PAGE TABLES ####
; #############################

align 4096
boot_page_table:
    times 256 dd 0
    dd boot_page + PAGE_ENTRY_BITS
    times (768 - 1) dd 0

align 4096
page_table:
    times (MAX_PAGE_TABLES * 1024) dd 0

align 4096
page_dir:
    dd boot_page_table + PAGE_ENTRY_BITS
    times (768 - 1) dd 0
    ; WHEN CHANGING MAX_PAGE_TABLES, MUST AJUST THE FOLLOWING ENTRIES
    dd page_table + 0x0000 + PAGE_ENTRY_BITS
    dd page_table + 0x1000 + PAGE_ENTRY_BITS
    dd page_table + 0x2000 + PAGE_ENTRY_BITS
    dd page_table + 0x3000 + PAGE_ENTRY_BITS

    times (256 - MAX_PAGE_TABLES) dd 0

; ###########################
; #### KERNEL CODE ENTRY ####
; ###########################

section .text

_entry:
    mov esp, stack_top
    mov ebp, esp
    
    call _init
    call kernel_main
    call _fini

    sti

loop:
    hlt
    jmp loop

section .data
stack_bottom:
    resb 4096 * 32
stack_top:
    dd 0x00000000