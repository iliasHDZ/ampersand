#pragma once

#include <common.h>
#include <data/chain.hpp>
#include <arch/thread.hpp>
#include <mem/paging.hpp>

#define DEFAULT_STACK_SIZE 64 * KiB

typedef void(*ThreadEntry)(void*);

class ThreadScheduler;

class Process;

class Thread : public DLChainItem {
private:
    Thread(ThreadScheduler* scheduler, VirtualMemory* vmem, Process* process = nullptr);

    virtual ~Thread();

    void await(ThreadSignal* signal);

    void emit(ThreadSignal* signal);

    bool is_blocked();

public:
    void NO_RETURN resume();

    u32 get_id();

    inline Process* get_process() { return process; };

    CPUState* get_cpu_state();

    CPUState* get_syscall_state();

private:
    ArchThreadInstance instance;

    VirtualMemory* vmem;

    Process* process;

    bool cpu_state_dirty = true;
    CPUState cpu_state;
    CPUState syscall_state;

    ThreadSignal* signal = nullptr;

    ThreadScheduler* scheduler;

    void* allocated_stack = nullptr;

    u32 id;

    friend class ThreadScheduler;
};

class ThreadScheduler {
public:
    ThreadScheduler();

    Thread* create_thread(ThreadEntry entry, void* param, usize stack_size = DEFAULT_STACK_SIZE);

    Thread* create_user_thread(ThreadEntry entry, Process* process, VirtualMemory* vmem, void* param, void* stack, usize stack_size);

    void exit(Thread* thread);

    void NO_RETURN run();

    void emit(ThreadSignal* signal);

    void handle_yield(ArchThreadYieldStatus status);

    void handle_exception(Exception* excpt);

    void init_scheduler();

    inline Thread* current() { return current_thread; };

public:
    static void init();

    static ThreadScheduler* get();

private:
    DLChain<Thread> threads;

    Thread* current_thread = nullptr;

    friend class Thread;
};

