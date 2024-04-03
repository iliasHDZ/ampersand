#include "manager.hpp"

#include <logger.hpp>
#include <data/thread.hpp>

static ProcessManager pm_instance;

static ThreadSignal close_process;
static Process* process_to_be_closed = nullptr;

Process* ProcessManager::get_process(usize pid) {
    for (auto& process : processes)
        if (process.get_pid() == pid)
            return &process;

    return nullptr;
}

Process* ProcessManager::get_process_with_vmem(VirtualMemory* vmem) {
    for (auto& process : processes)
        if (process.memory->get_vmem() == vmem)
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

Process* ProcessManager::fork(Process* process, Thread* caller) {
    Process* ret = process->fork(caller);
    insert(ret);

    Log::INFO("ProcessManager") << "Forked process " << ret->get_pid() << " from process " << process->get_pid() << '\n';

    return ret;
}

isize ProcessManager::syscall(Process* process, usize a, usize b, usize c, usize d) {
    Thread* prev = ThreadScheduler::get()->current();

    switch (a) {
    case SYSCALL_EXIT:
        run_extcmd(EXTCMD_EXIT, process);
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
    case SYSCALL_FORK: {
        isize ret = run_extcmd(EXTCMD_FORK, process);

        if (ProcessManager::get()->get_process_with_vmem(VirtualMemoryManager::get()->get_current()) != process)
            return 0;

        return ret;
    }
    case SYSCALL_EXEC:
        return process->sys_exec((const char*)b);
    case SYSCALL_POLL:
        return process->sys_poll((struct pollfd*)b, c, d);
    case SYSCALL_IOCTL:
        return process->sys_ioctl(b, c, (usize*)d);
    case SYSCALL_BRK:
        process->set_brk((void*)b);
        return 0;
    case SYSCALL_SBRK:
        process->set_brk((void*)((usize)process->brk + (usize)b));
        return (isize)process->brk;
    }

    return 0;
}

void ProcessManager::access_fault(Process* process, AccessFault fault) {
    AccessFaultAction action = process->get_memory()->access_fault(fault);

    if (action == AccessFaultAction::SEGFAULT) {
        run_extcmd(EXTCMD_EXIT, process);
        kthread_await(0);
    }
}

static bool extcmd_fork = false;

isize ProcessManager::handle_extcmd(usize cmd, Process* proc, Thread* caller, void* param) {
    switch (cmd) {
    case EXTCMD_EXIT:
        exit(proc);
        return 0;
    case EXTCMD_FORK:
        extcmd_fork = true;
        return fork(proc, caller)->get_pid();
    case EXTCMD_EXEC:
        proc->exec((FileDescription*)param);
        return 0;
    }
}

static ThreadSignal extcmd_run_signal;
static ThreadSignal extcmd_finish_signal;

static Mutex extcmd_mutex;

static usize    extcmd_cmd;
static Process* extcmd_proc;
static Thread*  extcmd_caller;
static void*    extcmd_param;
static bool     extcmd_finished;
static isize    extcmd_return;

isize ProcessManager::run_extcmd(usize cmd, Process* proc, void* param) {
    extcmd_mutex.lock();

    extcmd_cmd      = cmd;
    extcmd_proc     = proc;
    extcmd_caller   = ThreadScheduler::get()->current();
    extcmd_param    = param;
    extcmd_finished = false;

    kthread_emit(&extcmd_run_signal);

    while (!extcmd_finished)
        kthread_await(&extcmd_finish_signal);

    if (extcmd_fork)
        extcmd_fork = false;
    else
        extcmd_mutex.unlock();

    return extcmd_return;
}

static void process_extcmd_thread(void* _) {
    while (true) {
        kthread_await(&extcmd_run_signal);
        if (extcmd_finished)
            continue;

        extcmd_return   = ProcessManager::get()->handle_extcmd(extcmd_cmd, extcmd_proc, extcmd_caller, extcmd_param);
        extcmd_finished = true;

        kthread_emit(&extcmd_finish_signal);
    }
}

Mutex set_a_mutex;

static void process_syscall_handler() {
    Thread* current = ThreadScheduler::get()->current();
    if (current == nullptr)
        return;

    Process* process = current->get_process();
    if (process == nullptr)
        return;

    CPUState* state = current->get_syscall_state();

    usize ret = ProcessManager::get()->syscall(process, state->a(), state->b(), state->c(), state->d());

    state = ThreadScheduler::get()->current()->get_syscall_state();

    set_a_mutex.lock();
    state->set_a(ret);
    set_a_mutex.unlock();
}

void ProcessManager::init_manager() {
    kthread_create(process_extcmd_thread, nullptr);

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