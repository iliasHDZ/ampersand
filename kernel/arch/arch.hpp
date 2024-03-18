#pragma once

#include <common.h>

#ifdef ARCH_I386
#include "i386/src/arch.hpp"
#endif

extern const char* arch_reg_names[ARCH_REG_COUNT];

struct CPUState {
    usize regs[ARCH_REG_COUNT];

    usize a() const;
    usize b() const;
    usize c() const;
    usize d() const;

    usize pc() const;
    usize sp() const;
    usize bp() const;

    void log(bool err = false) const;
};

u8 arch_inb(u32 port);

u16 arch_inw(u32 port);

u32 arch_inl(u32 port);

void arch_outb(u32 port, u8 value);

void arch_outw(u32 port, u16 value);

void arch_outl(u32 port, u32 value);

void arch_idle_cpu();

void arch_lock_cpu();

void arch_init();