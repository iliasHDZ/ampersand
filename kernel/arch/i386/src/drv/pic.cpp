#include "pic.hpp"

#include "../io.h"

#define ICW1_ICW4	    0x01
#define ICW1_SINGLE	    0x02
#define ICW1_INTERVAL4	0x04
#define ICW1_LEVEL	    0x08
#define ICW1_INIT	    0x10
 
#define ICW4_8086	    0x01
#define ICW4_AUTO	    0x02
#define ICW4_BUF_SLAVE	0x08
#define ICW4_BUF_MASTER	0x0C
#define ICW4_SFNM	    0x10

#define PIC_EOI		    0x20

static PIC8259Pair pair_instance;

PIC8259::PIC8259(u8 io_offset, bool is_slave)
    : enabled(true), mask(0), io_offset(io_offset), is_slave(is_slave) {}

void PIC8259::command(u8 cmd) {
    outb(io_offset, cmd);
}

u8 PIC8259::read() {
    return inb(io_offset + 1);
}

void PIC8259::write(u8 value) {
    outb(io_offset + 1, value);
}

void PIC8259::eoi() {
    command(PIC_EOI);
}

void PIC8259::mask_irq(u32 irq) {
    mask |= 1 << irq;

    if (!enabled)
        return;

    write(mask);
}

void PIC8259::unmask_irq(u32 irq) {
    mask &= ~(1 << irq);

    if (!enabled)
        return;

    write(mask);
}

void PIC8259::disable() {
    write(0xff);
    enabled = false;
}

void PIC8259::enable() {
    write(mask);
    enabled = true;
}

void PIC8259::remap(u32 offset) {
    u8 a = read();

    command(ICW1_INIT | ICW1_ICW4);

    write(offset);

    if (is_slave)
        write(2);
    else
        write(4);

    write(ICW4_8086);
    write(a);

    irq_offset = offset;
}

u32 PIC8259::get_irq_offset() const {
    return irq_offset;
}

PIC8259Pair::PIC8259Pair()
    : master(PIC8259_MASTER_IO_OFFSET, false), slave(PIC8259_SLAVE_IO_OFFSET, true) {}

void PIC8259Pair::endOfInterrupt(u32 irq) {
    u32 mirq = master.get_irq_offset();
    u32 sirq = slave.get_irq_offset();

    if (irq >= mirq && irq < mirq + 8)
        master.eoi();

    if (irq >= sirq && irq < sirq + 8)
        slave.eoi();
}

void PIC8259Pair::mask(u32 irq) {
    if (irq < 8)
        master.mask_irq(irq);
    slave.mask_irq(irq - 8);
}

void PIC8259Pair::unmask(u32 irq) {
    if (irq < 8)
        master.unmask_irq(irq);
    slave.unmask_irq(irq - 8);
}

void PIC8259Pair::disable() {
    master.disable();
    slave.disable();
}

void PIC8259Pair::enable() {
    master.enable();
    slave.enable();
}

void PIC8259Pair::remap(u32 master_offset, u32 slave_offset) {
    master.remap(master_offset);
    slave.remap(slave_offset);
}

PIC8259Pair* PIC8259Pair::get() {
    return &pair_instance;
}