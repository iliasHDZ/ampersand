#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>

#include "prompt.hpp"

int main(int argc, char* argv[]) {
    CommandPrompt prompt(STDOUT_FILENO, STDIN_FILENO);

    while (true) {
        puts("prompt -> ");
        const char* cmd = prompt.prompt();
        printf("\n%s\n", cmd);
    }

    for (;;);
}