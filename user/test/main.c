#include <stdlib.h>

int _start() {
    syscall(0, 0, 0, 0);
    for (;;);
}