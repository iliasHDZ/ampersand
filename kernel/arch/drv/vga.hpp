#include <common.h>

namespace binding {

void vga_enable_cursor(u8 start_sl, u8 end_sl);

void vga_disable_cursor();

void vga_set_cursor_pos(u32 x, u32 y);

}