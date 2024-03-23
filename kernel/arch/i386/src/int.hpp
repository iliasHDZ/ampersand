#pragma once

#include <common.h>

#include <arch/arch.hpp>

#define INT_ID_COUNT    256
#define INT_EXCPT_COUNT 32

typedef void (*InterruptHandler)(void*);

struct PACKED_STRUCT int_regs {
    u32 ds;
    u32 edi, esi, ebp, esp, ebx, edx, ecx, eax;
    u32 int_no, err_code;
    u32 eip, cs, eflags, useresp, ss;
};

extern int_regs* int_isr_regs;

extern "C" void* syscall_handler_func;

void int_regs_to_cpu_state(CPUState* dst, int_regs* src);

bool int_register_irq(u32 id, InterruptHandler handler, void* param = nullptr);

extern "C" void int_isr_handler();

u32 int_get_esp();

u32 int_get_ebp();

void NO_RETURN int_return(u32 esp, u32 ebp);

void int_enable();

void int_disable();

bool int_init();