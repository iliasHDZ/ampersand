#include "thread.hpp"

#include "logger.hpp"

static u32 thread_id_counter = 0;

static ThreadScheduler scheduler_instance;

#if __INTELLISENSE__
#pragma diag_suppress 144
#endif

Thread::Thread(ThreadScheduler* scheduler)
    : scheduler(scheduler) {}

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
    cpu_state_dirty = true;

    arch_thread_resume(&instance);
}

u32 Thread::get_id() {
    return id;
}

ThreadScheduler::ThreadScheduler() {}

Thread* ThreadScheduler::create_thread(ThreadEntry entry, void* param, usize stack_size) {
    Thread* thread = new Thread(this);

    thread->id = thread_id_counter++;
    thread->allocated_stack = kmalloc(stack_size);

    arch_thread_create(&(thread->instance), (void*)entry, param, thread->allocated_stack, stack_size);

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