#include <stdlib.h>

unsigned int syscall(unsigned int a, unsigned int b, unsigned int c, unsigned int d) {
    asm("int $0xC0" : "=a" (a) : "a" (a), "b" (b), "c" (c), "d" (d));
    return a;
}