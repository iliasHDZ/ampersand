#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include "../libc/src/common.hpp"

#include "terminal.hpp"

#include "ansi.hpp"

int main() {
    VGATerminal term;
    ANSIParser ansi(&term);

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

    return 0;
}