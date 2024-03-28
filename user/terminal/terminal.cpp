#include "terminal.hpp"

#include <fcntl.h>

Terminal::~Terminal() {}

VGATerminal::VGATerminal() {
    fd = open("/dev/vga", O_RDWR);
}

VGATerminal::~VGATerminal() {
    close(fd);
}

void VGATerminal::put(char ch, char bg, char fg) {
    char buf[] = { ch, ((bg & 0xf) << 4) | (fg & 0xf) };
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
        line[i * 2 + 1] = ((bg & 0xf) << 4) | (fg & 0xf);
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