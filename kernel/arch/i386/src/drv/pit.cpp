#include "pit.hpp"

#include "../io.h"

void PIT8253::setup_channel(u8 ch, u8 mode, u16 divisor) {
    outb(PIT_COMMAND_PORT, (ch << 6) | (mode << 1) | 0x30);

    u16 ch_port = PIT_CHANNEL0_PORT + ch;
    outb(ch_port, divisor & 0xff);
    outb(ch_port, divisor >> 8);
}