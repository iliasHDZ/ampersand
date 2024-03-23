#pragma once

#include "process.hpp"

class ProcessManager {
public:
    Process* get_process(usize pid);

    Process* create();

public:
    static ProcessManager* get();

private:
    usize pid_counter = 1;

    DLChain<Process> processes;
};