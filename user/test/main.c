#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>

static char thing[] = { '#',  0x0f };

int _start() {
    int fd = open("/dev/vga", O_RDWR);
    if (fd == -1)
        exit(-1);

    for (int i = 0; i < 80 * 25; i++)
        write(fd, thing, 2);

    close(fd);
    exit(0);
}