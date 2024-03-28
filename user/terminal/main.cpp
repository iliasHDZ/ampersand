#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>

#include "terminal.hpp"

int main() {
    VGATerminal term;

    term.put('T', 0, 0xf);
    term.put('E', 0, 0xf);
    term.put('S', 0, 0xf);
    term.put('T', 0, 0xf);

    term.set_cursor(2, 2);

    term.put('L', 0, 0xf);
    term.put('M', 0, 0xf);
    term.put('A', 0, 0xf);
    term.put('O', 0, 0xf);

    term.scroll(0, 0);

    return 0;
}