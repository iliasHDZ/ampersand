#pragma once

#include "process.hpp"
#include <unistd.h>

#define EXTCMD_EXIT 0
#define EXTCMD_FORK 1
#define EXTCMD_EXEC 2

class ProcessManager {
public:
    Process* get_process(usize pid);

    Process* get_process_with_vmem(VirtualMemory* vmem);

    Process* create();

    void exit(Process* process, i32 status = 0);

    Process* fork(Process* process, Thread* caller);

    isize syscall(Process* process, usize a, usize b, usize c, usize d);

    void access_fault(Process* process, AccessFault fault);

    // Extern commands (extcmd) are commands that are to be run
    // outside the process thread because it might modify the thread.
    // .ex: exit, fork, exec
    isize handle_extcmd(usize cmd, Process* proc, Thread* caller, void* param);

    isize run_extcmd(usize cmd, Process* proc, void* param = nullptr);

private:
    friend class Thread;

    void init_manager();

    void insert(Process* process);

public:
    static ProcessManager* get();

    static void init();

private:
    usize pid_counter = 1;

    DLChain<Process> processes;
};