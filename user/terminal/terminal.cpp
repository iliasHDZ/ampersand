#include "terminal.hpp"

#include <fcntl.h>

#define VGA_BLACK        0x0
#define VGA_BLUE         0x1
#define VGA_GREEN        0x2
#define VGA_CYAN         0x3
#define VGA_RED          0x4
#define VGA_PURPLE       0x5
#define VGA_BROWN        0x6
#define VGA_GRAY         0x7
#define VGA_DARK_GRAY    0x8
#define VGA_LIGHT_BLUE   0x9
#define VGA_LIGHT_GREEN  0xA
#define VGA_LIGHT_CYAN   0xB
#define VGA_LIGHT_RED    0xC
#define VGA_LIGHT_PURPLE 0xD
#define VGA_YELLOW       0xE
#define VGA_WHITE        0xF

Terminal::~Terminal() {}

VGATerminal::VGATerminal() {
    fd = open("/dev/vga", O_RDWR);

    set_cursor(0, 0);
    for (int i = 0; i < 80 * 25; i++)
        put(' ', 0, 0);
    set_cursor(0, 0);
}

VGATerminal::~VGATerminal() {
    close(fd);
}

void VGATerminal::put(char ch, char bg, char fg) {
    char buf[] = { ch, map_color(bg, fg) };
    write(fd, buf, 2);
}

void VGATerminal::set_cursor(int x, int y) {
    lseek(fd, (y * get_width() + x) * 2, SEEK_SET);
}

void VGATerminal::scroll(char bg, char fg) {
    char line[80 * 2] = { 0 };

    off_t prev = lseek(fd, 0, SEEK_CUR);

    for (int i = 0; i < (get_height() - 1); i++) {
        lseek(fd, (i + 1) * get_width() * 2, SEEK_SET);
        read(fd, line, 80 * 2);
        lseek(fd, i * get_width() * 2, SEEK_SET);
        write(fd, line, 80 * 2);
    }

    for (int i = 0; i < get_width(); i++) {
        line[i * 2 + 0] = ' ';
        line[i * 2 + 1] = map_color(bg, fg);
    }

    write(fd, line, 80 * 2);
    lseek(fd, prev, SEEK_SET);
}

int VGATerminal::get_width() {
    return 80;
}

int VGATerminal::get_height() {
    return 25;
}

static char vga_color_map[] = {
    VGA_BLACK,
    VGA_RED,
    VGA_GREEN,
    VGA_BROWN,
    VGA_BLUE,
    VGA_PURPLE,
    VGA_CYAN,
    VGA_GRAY,
    VGA_DARK_GRAY,
    VGA_LIGHT_RED,
    VGA_LIGHT_GREEN,
    VGA_YELLOW,
    VGA_LIGHT_BLUE,
    VGA_LIGHT_PURPLE,
    VGA_LIGHT_CYAN,
    VGA_WHITE
};

char VGATerminal::map_color(char bg, char fg) {
    bg = vga_color_map[bg];
    fg = vga_color_map[fg];

    return ((bg & 0xf) << 4) | (fg & 0xf);
}