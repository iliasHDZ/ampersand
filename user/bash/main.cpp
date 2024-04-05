#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>

int main(int argc, char* argv[]) {
    puts("testings! \e[91mRED!\n");

    printf("This is a printf test -> %i\n", 62745);

    for (int i = 0; i < argc; i++) {
        puts(argv[i]);
        putchar('\n');
    }

    while (true) {
        char ch;
        read(STDIN_FILENO, &ch, 1);
        write(STDOUT_FILENO, &ch, 1);
    }

    for (;;);
}