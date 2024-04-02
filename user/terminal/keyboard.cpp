#include "keyboard.hpp"
#include <unistd.h>
#include <fcntl.h>

Keyboard::Keyboard() {
    fd = open("/dev/kbd", O_RDONLY | O_NONBLOCK);
}

Keyboard::~Keyboard() {
    close(fd);
}

void Keyboard::get_pollfd(struct pollfd* pfd) {
    pfd->fd      = fd;
    pfd->events  = POLLIN;
    pfd->revents = 0;
}

void Keyboard::update() {
    char buf[8];

    int ret = read(fd, buf, 8);
    if (ret < 0)
        return;
    
    for (int i = 0; i < ret; i++)
        handle_byte(buf[i] & 0xff);
}

void Keyboard::load_keymap(keymap* keym) {
    for (int i = 0; i < 128; i++)
        keyb_layout[i].scancode = 0;
    
    for (int i = 0; i < keym->count; i++) {
        keymapping* keymp_src = &(keym->mappings[i]);
        keymapping* keymp_dst = &(keyb_layout[keymp_src->scancode]);

        keymp_dst->scancode        = keymp_src->scancode;
        keymp_dst->character       = keymp_src->character;
        keymp_dst->character_shift = keymp_src->character_shift == 0 ? keymp_src->character : keymp_src->character_shift;
        keymp_dst->character_alt   = keymp_src->character_alt;
    }
}

void Keyboard::handle_byte(unsigned char ch) {
    if (extended_enabled) {
        handle_scancode(0xe000 | (int)ch);
        extended_enabled = false;
        return;
    }

    if (ch == 0xe0) {
        extended_enabled = true;
    } else
        handle_scancode(ch);
}

void Keyboard::handle_scancode(int sc) {
    switch (sc) {
    case KEY_UP:
        write("\e[A", 3);
        return;
    case KEY_DOWN:
        write("\e[B", 3);
        return;
    case KEY_RIGHT:
        write("\e[C", 3);
        return;
    case KEY_LEFT:
        write("\e[D", 3);
        return;
    case KEY_BACKSPACE:
        write("\b", 1);
        return;
    case KEY_ENTER:
        write("\n", 1);
        return;
    case KEY_TAB:
        write("\t", 1);
        return;
    case KEY_ESC:
        write("\e", 1);
        return;
    }

    if ((sc >> 8) == 0xe0)
        return;
    
    char key     = sc & 0x7f;
    bool pressed = !((sc >> 7) & 1);

    switch (key) {
    case KEY_LALT:
        alt = pressed;
        return;
    case KEY_LCTRL:
        ctrl = pressed;
        return;
    case KEY_LSHIFT:
        lshift = pressed;
        return;
    case KEY_RSHIFT:
        rshift = pressed;
        return;
    case KEY_CAPSLOCK:
        if (pressed)
            capslock = !capslock;
        return;
    }

    if (!pressed)
        return;

    int prs = 0;
    if (alt) prs++;
    if (ctrl) prs++;
    if (lshift || rshift) prs++;
    if (prs > 1)
        return;

    keymapping* map = &keyb_layout[key];
    if (map->scancode != key)
        return;

    bool shift = lshift || rshift;
    if (capslock)
        shift = !shift;
    
    if (shift) {
        if (map->character_shift == 0)
            return;

        write(&map->character_shift, 1);
        return;
    }
    
    if (alt) {
        if (map->character_alt == 0)
            return;

        write(&map->character_alt, 1);
        return;
    }

    if (map->character == 0)
        return;
    write(&map->character, 1);
}

void Keyboard::write(char* buf, int size) {
    if (write_callback)
        write_callback(buf, size);
}