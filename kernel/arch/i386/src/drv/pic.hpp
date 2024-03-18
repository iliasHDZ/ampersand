#include <common.h>

#define PIC8259_MASTER_IO_OFFSET 0x20
#define PIC8259_SLAVE_IO_OFFSET  0xa0

class PIC8259 {
public:
    PIC8259(u8 io_offset, bool is_slave);

    void command(u8 cmd);

    u8 read();
    void write(u8 value);

    void eoi();

    void mask_irq(u32 irq);
    void unmask_irq(u32 irq);

    void disable();
    void enable();

    void remap(u32 offset);

    u32 get_irq_offset() const;

private:
    bool enabled;
    u8   mask;
    
    u8   io_offset;
    bool is_slave;

    u32  irq_offset;

};

class PIC8259Pair {
public:
    PIC8259Pair();

    void endOfInterrupt(u32 irq);

    void mask(u32 irq);
    void unmask(u32 irq);

    void disable();
    void enable();

    void remap(u32 master_offset, u32 slave_offset);

public:
    static PIC8259Pair* get();

private:
    PIC8259 master;
    PIC8259 slave;
};