#pragma once

#include "memory.hpp"

#include "thread.hpp"
#include <data/chain.hpp>
#include <fd/fd.hpp>
#include "error.hpp"

#define PROCESS_STACK_SIZE (128 * KiB)

class Process : public DLChainItem {
public:
    Process(usize pid);

    ~Process();

    inline usize get_pid() { return pid; };

    SyscallError exec(FileDescription* file);

private:
    Vec<Thread*> threads;

    usize pid;
    ProcessMemory* memory;
};