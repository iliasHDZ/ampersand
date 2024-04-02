#pragma once

#include <poll.h>
#include "keys.hpp"

typedef void(*KeyboardWriteCallback)(char*, int);

class Keyboard {
public:
    Keyboard();

    virtual ~Keyboard();

    void get_pollfd(struct pollfd* pfd);

    void update();

    void load_keymap(keymap* keym);

    inline void set_write_callback(KeyboardWriteCallback cb) { write_callback = cb; };

private:
    void handle_byte(unsigned char ch);

    void handle_scancode(int sc);

    void write(char* buf, int size);

private:
    int fd;

    bool extended_enabled = false;

    bool alt      = false;
    bool ctrl     = false;
    bool lshift   = false;
    bool rshift   = false;
    bool capslock = false;

    KeyboardWriteCallback write_callback = nullptr;

    keymapping keyb_layout[128];

};