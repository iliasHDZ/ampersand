#pragma once

#include "common.h"

#include "data/chain.hpp"

#include <arch/thread.hpp>

#define DEFAULT_STACK_SIZE 64 * KiB

typedef void(*ThreadEntry)(void*);

class ThreadScheduler;

class Thread : public DLChainItem {
private:
    Thread(ThreadScheduler* scheduler);

    virtual ~Thread();

    CPUState* get_cpu_state();

    void await(ThreadSignal* signal);

    void emit(ThreadSignal* signal);

    bool is_blocked();

public:
    void NO_RETURN resume();

    u32 get_id();

private:
    ArchThreadInstance instance;

    bool cpu_state_dirty = true;
    CPUState cpu_state;

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

    void exit(Thread* thread);

    void NO_RETURN run();

    void emit(ThreadSignal* signal);

    void handle_yield(ArchThreadYieldStatus status);

    void handle_exception(Exception* excpt);

    void init_scheduler();

public:
    static void init();

    static ThreadScheduler* get();

private:
    DLChain<Thread> threads;

    Thread* current_thread = nullptr;

    friend class Thread;
};

