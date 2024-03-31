#pragma once

#include <unistd.h>

typedef unsigned int u32;

inline u32 _kernel_syscall(u32 a, u32 b, u32 c, u32 d) {
    asm("int $0xC0" : "=a" (a) : "a" (a), "b" (b), "c" (c), "d" (d));
    return a;
}