#include <arch/drv/vga.hpp>
#include "../io.h"

namespace binding {

void vga_enable_cursor(u8 start_sl, u8 end_sl) {
    outb(0x3D4, 0x0A);
	outb(0x3D5, (inb(0x3D5) & 0xC0) | start_sl);
 
	outb(0x3D4, 0x0B);
	outb(0x3D5, (inb(0x3D5) & 0xE0) | end_sl);
}

void vga_disable_cursor() {
    outb(0x3D4, 0x0A);
	outb(0x3D5, 0x20);
}

void vga_set_cursor_pos(u32 x, u32 y) {
    u16 pos = y * 80 + x;
 
	outb(0x3D4, 0x0F);
	outb(0x3D5, (u8) (pos & 0xFF));
	outb(0x3D4, 0x0E);
	outb(0x3D5, (u8) ((pos >> 8) & 0xFF));
}

}