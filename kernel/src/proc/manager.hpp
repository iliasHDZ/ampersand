#pragma once

#include "process.hpp"

#define SYSCALL_EXIT 0

class ProcessManager {
public:
    Process* get_process(usize pid);

    Process* create();

    void exit(Process* process);

    usize syscall(Process* process, usize a, usize b, usize c, usize d);

private:
    friend class Thread;

    void init_manager();

public:
    static ProcessManager* get();

    static void init();

private:
    usize pid_counter = 1;

    DLChain<Process> processes;
};