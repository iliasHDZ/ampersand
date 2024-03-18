#include <arch/arch.hpp>
#include "int.hpp"
#include "thread.hpp"
#include "timer.hpp"
#include "drv/pic.hpp"
#include "io.h"

#include <logger.hpp>
#include <thread.hpp>

bool arch_cpu_is_idling  = false;
bool arch_cpu_was_idling = false;

u8 arch_inb(u32 port) {
    return inb(port);
}

u16 arch_inw(u32 port) {
    return inw(port);
}

u32 arch_inl(u32 port) {
    return inl(port);
}

void arch_outb(u32 port, u8 value) {
    outb(port, value);
}

void arch_outw(u32 port, u16 value) {
    outw(port, value);
}

void arch_outl(u32 port, u32 value) {
    outl(port, value);
}

const char* arch_reg_names[ARCH_REG_COUNT] = {
    "eax",
    "ecx",
    "edx",
    "ebx",
    "esp",
    "ebp",
    "esi",
    "edi",
    "cs",
    "ds",
    "ss",
    nullptr,
    "eip",
    "eflags"
};

usize CPUState::a() const {
    return regs[ARCH_REG_EAX];
}

usize CPUState::b() const {
    return regs[ARCH_REG_EBX];
}

usize CPUState::c() const {
    return regs[ARCH_REG_ECX];
}

usize CPUState::d() const {
    return regs[ARCH_REG_EDX];
}

usize CPUState::pc() const {
    return regs[ARCH_REG_EIP];
}

usize CPUState::sp() const {
    return regs[ARCH_REG_ESP];
}

usize CPUState::bp() const {
    return regs[ARCH_REG_EBP];
}

void CPUState::log(bool err) const {
    OutputStream& o = (err ? Log::ERR() : Log::INFO()) << Out::phex(8);

    o << "eip = " << regs[ARCH_REG_EIP] << ", eflags = " << regs[ARCH_REG_EFLAGS] << '\n';

    o = (err ? Log::ERR() : Log::INFO()) << Out::phex(8);
    o <<   "eax = " << regs[ARCH_REG_EAX] << ", ecx = " << regs[ARCH_REG_ECX];
    o << ", edx = " << regs[ARCH_REG_EDX] << ", ebx = " << regs[ARCH_REG_EBX] << '\n';

    o = (err ? Log::ERR() : Log::INFO()) << Out::phex(8);
    o <<   "esp = " << regs[ARCH_REG_ESP] << ", ebp = " << regs[ARCH_REG_EBP];
    o << ", esi = " << regs[ARCH_REG_ESI] << ", edi = " << regs[ARCH_REG_EDI] << '\n';

    o = (err ? Log::ERR() : Log::INFO()) << Out::phex(8);
    o <<   "cs = " << regs[ARCH_REG_CS] << ", ds = " << regs[ARCH_REG_DS];
    o << ", ss = " << regs[ARCH_REG_SS] << '\n';
}

void arch_idle_cpu() {
    arch_cpu_is_idling = true;

    int_enable();

    while (1)
            asm("hlt");
}

void arch_lock_cpu() {
    int_disable();

    for (;;);
}

ThreadSignal keyboard_input_signal;

void arch_init() {
    auto pic = PIC8259Pair::get();

    pic->remap(32, 40);

    if (!int_init())
        panic("Failed to initialize i386 interrupts");

    Timer::init();

    arch_thread_init();

    int_register_irq(1, [](void*) {
        inb(0x60);
        kthread_emit(&keyboard_input_signal);
    });
}