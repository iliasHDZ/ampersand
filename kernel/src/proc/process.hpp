#pragma once

#include "memory.hpp"

#include "thread.hpp"
#include <data/chain.hpp>
#include <fd/fd.hpp>
#include "error.hpp"

#define PROCESS_STACK_SIZE (128 * KiB)

class ProcessManager;

class Process : public DLChainItem {
public:
    Process();

    ~Process();

    inline usize get_pid() { return pid; };

    SyscallError exec(FileDescription* file);

private:
    void close();

    void set_pid(usize pid);

private:
    Vec<Thread*> threads;

    usize pid = 0;
    ProcessMemory* memory;

    friend class ProcessManager;
};