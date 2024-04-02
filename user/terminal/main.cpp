#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include "../libc/src/common.hpp"

#include "terminal.hpp"
#include "ansi.hpp"
#include "keyboard.hpp"
#include "keys_be.hpp"

ANSIParser* ansip = nullptr;

static void on_keyboard_write(char* data, int size) {
    ansip->write(data, size);
}

int main() {
    VGATerminal term;
    ANSIParser ansi(&term);
    Keyboard keyb;

    ansip = &ansi;

    struct pollfd pfds[1];

    keyb.get_pollfd(pfds);
    keyb.set_write_callback(on_keyboard_write);
    keyb.load_keymap(&keymap_be);

    while (true) {
        keyb.update();
        poll(pfds, 1, 1000);
    }

    /*
    int stdout_pipe[2];
    if (pipe(stdout_pipe) < 0)
        return -1;
    
    if (fork() == 0) {
        dup2(stdout_pipe[1], STDOUT_FILENO);

        exec("/bin/bash");
    } else {
        char ch;

        while (true) {
            read(stdout_pipe[0], &ch, 1);

            ansi.put(ch);
        }
    }
    */

    return 0;
}