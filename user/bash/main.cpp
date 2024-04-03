#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>

int main() {
    puts("testings! \e[91mRED!\n");

    printf("This is a printf test -> %i", 62745);

    while (true) {
        char ch;
        read(STDIN_FILENO, &ch, 1);
        write(STDOUT_FILENO, &ch, 1);
    }

    for (;;);
}