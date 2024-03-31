#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include "../libc/src/common.hpp"

#include "terminal.hpp"

char child[]  = { 'C', 0x0f };
char parent[] = { 'P', 0x0f };

int main() {
    VGATerminal term;

    term.set_cursor(0, 0);
    for (int i = 0; i < 80 * 25; i++)
        term.put(' ', 0, 0xf);

    term.set_cursor(0, 0);
    term.put('T', 0, 0xf);
    term.put('E', 0, 0xf);
    term.put('S', 0, 0xf);
    term.put('T', 0, 0xf);

    if (fork() == 0) {
        term.set_cursor(2, 2);

        term.put('C', 0, 0xf);
        term.put('H', 0, 0xf);
        term.put('I', 0, 0xf);
        term.put('L', 0, 0xf);
        term.put('D', 0, 0xf);
    } else {
        term.set_cursor(4, 4);

        term.put('P', 0, 0xf);
        term.put('A', 0, 0xf);
        term.put('R', 0, 0xf);
        term.put('E', 0, 0xf);
        term.put('N', 0, 0xf);
        term.put('T', 0, 0xf);
    }
    
    /*int fd = open("/dev/vga", O_RDWR);
    if (fd == -1)
        return -1;

    if (fork() == 0) {
        lseek(fd, 2, SEEK_SET);
        write(fd, child, 2);
    } else {
        write(fd, parent, 2);
    }

    close(fd);*/

    return 0;
}