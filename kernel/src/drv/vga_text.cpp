#include "vga_text.hpp"

#include <arch/drv/vga.hpp>
#include <mem/paging.hpp>
#include <fs/devfs.hpp>

static u8* vga_framebuffer = 0;

static VGATextDriver vga_instance;

u32 VGATextDriver::get_rows() const {
    return 25;
}

u32 VGATextDriver::get_columns() const {
    return 80;
}

void VGATextDriver::fill(char ch, u8 bg, u8 fg) {
    u8* fb = vga_framebuffer;
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

    u8* fb = vga_framebuffer;
    fb[i + 0] = ch;
    fb[i + 1] = (bg << 4) | fg;
}

void VGATextDriver::set_cursor_raw(u32 x, u32 y) {
    index = y * get_width() + x;
    binding::vga_set_cursor_pos(x, y);
}

void VGATextDriver::scroll_raw(u32 lines, u8 bg, u8 fg) {
    u8* fb = vga_framebuffer;
    
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

void VGATextDriver::init() {
    vga_framebuffer = VirtualMemoryManager::get()->driver_map(0xb8000, 80 * 25 * 2);
}

VGATextDriver* VGATextDriver::get() {
    return &vga_instance;
}

static VGABlockDevice vga_blkdev;

void* VGABlockDevice::get_address() {
    return vga_framebuffer;
}

u64 VGABlockDevice::get_size() {
    return 80 * 25 * 2;
}

void VGABlockDevice::init() {
    DevFileSystem::get()->add_device(&vga_blkdev, DevInode::BLOCK_DEVICE, "vga");
}