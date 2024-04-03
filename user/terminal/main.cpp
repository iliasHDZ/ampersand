#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>

#include "terminal.hpp"
#include "ansi.hpp"
#include "keyboard.hpp"
#include "keys_be.hpp"

int keyboard_out_fd = -1;

static void on_keyboard_write(char* data, int size) {
    if (keyboard_out_fd != -1)
        write(keyboard_out_fd, data, size);
}

int main() {
    VGATerminal term;
    ANSIParser ansi(&term);
    Keyboard keyb;

    struct pollfd pfds[2];

    keyb.get_pollfd(&pfds[0]);
    keyb.set_write_callback(on_keyboard_write);
    keyb.load_keymap(&keymap_be);

    int stdout_pipe[2];
    if (pipe(stdout_pipe) < 0)
        return -1;

    int stdin_pipe[2];
    if (pipe(stdin_pipe) < 0)
        return -1;

    pfds[1].fd      = stdout_pipe[0];
    pfds[1].events  = POLLIN;
    pfds[1].revents = 0;
    
    if (fork() == 0) {
        dup2(stdin_pipe[0],  STDIN_FILENO);
        dup2(stdout_pipe[1], STDOUT_FILENO);

        exec("/bin/bash");
    } else {
        char ch;
        keyboard_out_fd = stdin_pipe[1];

        while (true) {
            poll(pfds, 2, 1000);

            if (pfds[0].revents)
                keyb.update();

            if (pfds[1].revents) {
                read(stdout_pipe[0], &ch, 1);
                ansi.put(ch);
            }
        }
    }

    return 0;
}