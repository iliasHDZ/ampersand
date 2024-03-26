#pragma once

#include <unistd.h>

typedef unsigned int u32;

u32 _kernel_syscall(u32 a, u32 b, u32 c, u32 d);