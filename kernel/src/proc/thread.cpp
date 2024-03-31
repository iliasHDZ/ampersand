#include "thread.hpp"

#include <logger.hpp>
#include "process.hpp"
#include "manager.hpp"

static u32 thread_id_counter = 0;

static ThreadScheduler scheduler_instance;

#if __INTELLISENSE__
#pragma diag_suppress 144
#endif

Thread::Thread(ThreadScheduler* scheduler, VirtualMemory* vmem, Process* process)
    : scheduler(scheduler), vmem(vmem), process(process) {}

Thread::Thread(Thread* src, VirtualMemory* vmem, Process* proc) {
    instance        = src->instance;
    vmem            = vmem;
    process         = proc;
    cpu_state_dirty = src->cpu_state_dirty;
    cpu_state       = src->cpu_state;
    syscall_state   = src->syscall_state;
    signal          = src->signal;
    scheduler       = src->scheduler;
}

Thread::~Thread() {
    if (scheduler->current_thread == this)
        scheduler->current_thread = nullptr;

    if (allocated_stack)
        kfree(allocated_stack);
}

CPUState* Thread::get_cpu_state() {
    if (cpu_state_dirty) {
        arch_thread_get_cpu_state(&instance, &cpu_state);
        cpu_state_dirty = false;    
    }

    return &cpu_state;
}

CPUState* Thread::get_syscall_state() {
    return &syscall_state;
}

void Thread::await(ThreadSignal* signal) {
    this->signal = signal;
}

void Thread::emit(ThreadSignal* signal) {
    if (this->signal == signal)
        this->signal = nullptr;
}

bool Thread::is_blocked() {
    return this->signal != nullptr;
}

void NO_RETURN Thread::resume() {
    VirtualMemoryManager::get()->use(vmem);

    cpu_state_dirty = true;
    arch_thread_resume(&instance);
}

u32 Thread::get_id() {
    return id;
}

ThreadScheduler::ThreadScheduler() {}

Thread* ThreadScheduler::create_thread(ThreadEntry entry, void* param, usize stack_size) {
    Thread* thread = new Thread(this, VirtualMemory::get_kernel_memory());

    thread->id = thread_id_counter++;
    thread->allocated_stack = kmalloc(stack_size);

    Log::INFO("ThreadScheduler") << "Created kernel thread " << thread->id << '\n';

    arch_thread_create(&(thread->instance), (void*)entry, param, thread->allocated_stack, stack_size);

    threads.append(thread);
    return thread;
}

Thread* ThreadScheduler::create_user_thread(ThreadEntry entry, Process* process, VirtualMemory* vmem, void* param, void* stack, usize stack_size) {
    Thread* thread = new Thread(this, vmem, process);
    
    thread->id = thread_id_counter++;

    Log::INFO("ThreadScheduler") << "Created user thread " << thread->id << " for process " << thread->id << '\n';

    arch_thread_create(&(thread->instance), (void*)entry, param, stack, stack_size);

    threads.append(thread);
    return thread;
}

Thread* ThreadScheduler::fork(Thread* source, VirtualMemory* vmem, Process* proc) {
    Thread* thread = new Thread(source, vmem, proc);

    thread->vmem = vmem;
    
    thread->id = thread_id_counter++;

    Log::INFO("ThreadScheduler") << "Forked thread " << thread->id << " from thread " << source->id << '\n';

    threads.append(thread);
    return thread;
}

void ThreadScheduler::exit(Thread* thread) {
    if (thread == current_thread)
        current_thread = nullptr;

    if (threads.remove(thread))
        delete thread;
}

void NO_RETURN ThreadScheduler::run() {
    if (current_thread == nullptr)
        current_thread = threads.first();
    
    if (current_thread == nullptr) {
        arch_idle_cpu();
        return;
    }

    auto thread = threads.after(current_thread);

    while (thread->is_blocked() && thread != current_thread) {
        thread = threads.after(thread);
    }

    if (thread->is_blocked()) {
        arch_idle_cpu();
        return;
    }

    current_thread = thread;
    thread->resume();
}

void ThreadScheduler::emit(ThreadSignal* signal) {
    for (auto& thread : threads)
        thread.emit(signal);
}

void ThreadScheduler::handle_yield(ArchThreadYieldStatus status) {
    current_thread->cpu_state_dirty = true;
    current_thread->vmem = VirtualMemoryManager::get()->get_current();

    if (status == ArchThreadYieldStatus::TIMER) {
        run();
        return;
    }

    if (status == ArchThreadYieldStatus::AWAIT) {
        CPUState* state = current_thread->get_cpu_state();
        current_thread->await((ThreadSignal*)state->a());
        run();
        return;
    }

    if (status == ArchThreadYieldStatus::SYSCALL) {
        current_thread->get_cpu_state();
        current_thread->syscall_state = current_thread->cpu_state;
        return;
    }

    if (status == ArchThreadYieldStatus::THREAD_EXIT) {
        exit(current_thread);
        current_thread = nullptr;
        run();
        return;
    }

    current_thread->resume();
}

void ThreadScheduler::handle_exception(Exception* excpt) {
    Log::ERR() << "KERNEL PANIC!\n";
    Log::ERR() << '\n';
    Log::ERR() << "Exception: " << excpt->get_name() << '\n';
    Log::ERR() << "In Thread " << current_thread->get_id() << '\n';
    Log::ERR() << '\n';

    excpt->get_state()->log(true);

    usize* bp = (usize*)excpt->get_state()->bp();

    Log::ERR() << "Calltrace:\n";

    while (bp && *bp) {
        Log::ERR() << Out::phex(8) << "  - " << bp[1] << '\n';
        bp = (usize*)*bp;
    }

    Log::ERR() << '\n';
    Log::ERR() << "Ampersand & v" << AMPERSAND_VERSION << '\n';
    Log::ERR() << '\n';
    Log::ERR() << "Please report this error to the developer\n";
    Log::ERR() << "You can manually restart or shutdown your computer\n";
    Log::ERR() << '\n';

    arch_lock_cpu();
}

static void thread_exception_handler(Exception* excpt) {
    scheduler_instance.handle_exception(excpt);
}

static void thread_yield_handler(ArchThreadYieldStatus status) {
    scheduler_instance.handle_yield(status);
}

void ThreadScheduler::init_scheduler() {
    arch_thread_set_exception_handler(thread_exception_handler);

    arch_thread_set_yield_handler(thread_yield_handler);
}

void ThreadScheduler::init() {
    scheduler_instance.init_scheduler();
}

ThreadScheduler* ThreadScheduler::get() {
    return &scheduler_instance;
}