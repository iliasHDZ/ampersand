#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include "../libc/src/common.hpp"

#include "terminal.hpp"

#include "ansi.hpp"

char child[]  = { 'C', 0x0f };
char parent[] = { 'P', 0x0f };

int main() {
    VGATerminal term;
    ANSIParser ansi(&term);

    ansi.write("This is a test! \e[91mThis is in red!\n\n\nsrthsrthsrthsrt\nhsrthsrths\n\n");
    ansi.write("This is a test! \e[91mThis is in red!\n\n\nsrthsrthsrthsrt\nhsrthsrths\n\n");
    ansi.write("This is a test! \e[91mThis is in red!\n\n\nsrthsrthsrthsrt\nhsrthsrths\n\n");
    ansi.write("This is a test! \e[91mThis is in red!\n\n\nsrthsrthsrthsrt\nhsrthsrths\na\na\n");

    /*
    int fd = open("/dev/kbd", O_RDONLY);

    while (true) {
        char c;
        read(fd, &c, 1);
        term.put('T', 0, 0xf);
    }
    */
    
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