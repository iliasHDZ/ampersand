#include <stdio.h>
#include <unistd.h>
#include <string.h>

int putchar(int ch) {
    char c = ch;
    return write(STDOUT_FILENO, &c, 1);
}

int puts(const char* str) {
    return write(STDOUT_FILENO, str, strlen(str));
}