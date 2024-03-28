#include "manager.hpp"

#include <logger.hpp>

static ProcessManager pm_instance;

static ThreadSignal close_process;
static Process* process_to_be_closed = nullptr;

Process* ProcessManager::get_process(usize pid) {
    for (auto& process : processes)
        if (process.get_pid() == pid)
            return &process;

    return nullptr;
}

Process* ProcessManager::create() {
    Process* process = new Process();
    insert(process);

    Log::INFO("ProcessManager") << "Created process " << process->get_pid() << '\n';

    return process;
}

void ProcessManager::exit(Process* process) {
    process->close();

    Log::INFO("ProcessManager") << "Closed process " << process->get_pid() << '\n';

    delete process;
}

isize ProcessManager::syscall(Process* process, usize a, usize b, usize c, usize d) {
    switch (a) {
    case SYSCALL_EXIT:
        process_to_be_closed = process;
        kthread_emit(&close_process);
        kthread_await(0);
        return 0;
    case SYSCALL_READ:
        return process->sys_read((isize)b, (void*)c, d);
    case SYSCALL_WRITE:
        return process->sys_write((isize)b, (void*)c, d);
    case SYSCALL_OPEN:
        return process->sys_open((const char*)b, c);
    case SYSCALL_CLOSE:
        return process->sys_close(b);
    case SYSCALL_LSEEK:
        return process->sys_lseek(b, c, d);
    case SYSCALL_PIPE:
        return process->sys_pipe((i32*)b);
    case SYSCALL_DUP:
        return process->sys_dup(b);
    case SYSCALL_DUP2:
        return process->sys_dup2(b, c);
    case SYSCALL_IOCTL:
        return process->sys_ioctl(b, c, (usize*)d);
    }

    return 0;
}

static void process_closer_thread(void* _) {
    while (true) {
        kthread_await(&close_process);

        if (process_to_be_closed == nullptr)
            continue;

        ProcessManager::get()->exit(process_to_be_closed);
        process_to_be_closed = nullptr;
    }
}

static void process_syscall_handler() {
    Thread* current = ThreadScheduler::get()->current();
    if (current == nullptr)
        return;

    Process* process = current->get_process();
    if (process == nullptr)
        return;

    CPUState* state = current->get_syscall_state();

    usize ret = ProcessManager::get()->syscall(process, state->a(), state->b(), state->c(), state->d());

    state->set_a(ret);
}

void ProcessManager::init_manager() {
    kthread_create(process_closer_thread, nullptr);

    arch_thread_set_syscall_handler(process_syscall_handler);
}

void ProcessManager::insert(Process* process) {
    usize pid = 1;

    for (auto& proc : processes) {
        if (pid == proc.get_pid()) {
            pid++;
            continue;
        }

        process->set_pid(pid);
        processes.insert_before(&proc, process);
        
        return;
    }

    process->set_pid(pid);
    processes.append(process);
}

ProcessManager* ProcessManager::get() {
    return &pm_instance;
}

void ProcessManager::init() {
    pm_instance.init_manager();
}