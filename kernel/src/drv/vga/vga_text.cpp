#include "vga_text.hpp"
#include "vga.hpp"

#include <arch/drv/vga.hpp>

VideoDevice* VGATextDriver::get_parent() const {
    return parent;
}

bool VGATextDriver::is_enabled() const {
    return get_current_text_mode() != nullptr;
}

u32 VGATextDriver::get_rows() const {
    auto mode = get_current_text_mode();
    if (mode == nullptr)
        return 0;

    return mode->rows;
}

u32 VGATextDriver::get_columns() const {
    auto mode = get_current_text_mode();
    if (mode == nullptr)
        return 0;

    return mode->columns;
}

u32 VGATextDriver::get_bytes_per_char() const {
    if (!is_enabled()) return 0;

    return 2;
}

usize VGATextDriver::get_column_stride() const {
    if (!is_enabled()) return 0;

    return get_columns() * get_bytes_per_char();
}

u8* VGATextDriver::get_framebuffer() const {
    if (!is_enabled()) return 0;

    return (u8*)(0xb8000);
}

void VGATextDriver::fill(char ch, u8 bg, u8 fg) {
    u8* fb = get_framebuffer();
    u8* fb_end = fb + get_width() * get_height() * 2;

    for (; fb < fb_end; fb += 2) {
        fb[0] = ch;
        fb[1] = (bg << 4) | fg;
    }
}

void VGATextDriver::set_cursor_visible(bool visible) {
    if (visible) {
        binding::vga_enable_cursor(14, 15);
    } else {
        binding::vga_disable_cursor();
    }
}

void VGATextDriver::put_raw(char ch, u8 bg, u8 fg) {
    usize i = index * 2;

    u8* fb = get_framebuffer();
    fb[i + 0] = ch;
    fb[i + 1] = (bg << 4) | fg;
}

void VGATextDriver::set_cursor_raw(u32 x, u32 y) {
    index = y * get_width() + x;
    binding::vga_set_cursor_pos(x, y);
}

void VGATextDriver::scroll_raw(u32 lines, u8 bg, u8 fg) {
    u8* fb = (u8*)get_framebuffer();
    
    u32 width  = get_width();
    u32 height = get_height();

    for (u32 y = 0; y < height; y++) {
        u32 sy = y + lines;

        if (sy >= height) {
            for (u32 x = 0; x < width; x++) {
                u32 idx = (y * width + x) * 2;
                fb[idx + 0] = ' ';
                fb[idx + 1] = (bg << 4) | fg;
            }
        } else {
            memcpy(&fb[y * width * 2], &fb[sy * width * 2], width * 2);
        }
    }
}
    
bool VGATextDriver::guide_cursor() {
    return true;
}

const VideoTextMode* VGATextDriver::get_current_text_mode() const {
    if (parent == nullptr)
        return nullptr;

    auto mode = parent->get_current_video_mode();
    
    if (!mode->is_text_mode)
        return nullptr;

    return (VideoTextMode*)mode;
}