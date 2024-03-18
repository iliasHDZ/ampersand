#pragma once

#include <common.h>

#define ARCH_REG_COUNT 16

#define ARCH_REG_EAX 0
#define ARCH_REG_ECX 1
#define ARCH_REG_EDX 2
#define ARCH_REG_EBX 3
#define ARCH_REG_ESP 4
#define ARCH_REG_EBP 5
#define ARCH_REG_ESI 6
#define ARCH_REG_EDI 7

#define ARCH_REG_CS  8
#define ARCH_REG_DS  9
#define ARCH_REG_SS  10

#define ARCH_REG_EIP 12
#define ARCH_REG_EFLAGS 13

#define ARCH_PAGE_SIZE 4096

struct ArchThreadInstance {
    bool first_run;
    usize entry;
    void* param;
    usize esp;
    usize ebp;
};

extern bool arch_cpu_is_idling;
extern bool arch_cpu_was_idling;