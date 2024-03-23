#include "manager.hpp"

#include <logger.hpp>

static ProcessManager pm_instance;

Process* ProcessManager::get_process(usize pid) {
    for (auto& process : processes)
        if (process.get_pid() == pid)
            return &process;

    return nullptr;
}

Process* ProcessManager::create() {
    Process* process = new Process(pid_counter++);

    Log::INFO("ProcessManager") << "Created process " << process->get_pid() << '\n';

    processes.append(process);

    return process;
}

ProcessManager* ProcessManager::get() {
    return &pm_instance;
}