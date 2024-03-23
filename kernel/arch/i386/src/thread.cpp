#include "thread.hpp"
#include "int.hpp"
#include "timer.hpp"
#include "logger.hpp"

usize thread_run_entry = 0;
usize thread_run_param = 0;
usize thread_run_esp   = 0;
usize thread_run_ebp   = 0;

// present in arch/i386/asm/thread.asm
extern "C" void NO_RETURN thread_run();

static ArchThreadYieldHandler yield_handler = nullptr;

static ArchThreadSyscallHandler syscall_handler = nullptr;

static ExceptionHandler excpt_handler = nullptr;

static ArchThreadInstance* current_running_thread = nullptr;

void arch_thread_set_yield_handler(ArchThreadYieldHandler handler) {
    yield_handler = handler;
}

void arch_thread_set_exception_handler(ExceptionHandler handler) {
    excpt_handler = handler;
}

void arch_thread_set_syscall_handler(ArchThreadSyscallHandler handler) {
    syscall_handler = handler;
}

void arch_thread_get_cpu_state(ArchThreadInstance* ins, CPUState* state) {
    int_regs_to_cpu_state(state, (int_regs*)ins->esp);
}

void arch_thread_create(ArchThreadInstance* ins, void* entry, void* param, void* stack, usize stack_size) {
    ins->first_run = true;
    ins->entry = (usize)entry;
    ins->param = param;

    usize* stack_top = (usize*)((usize)stack + stack_size);

    ins->esp = (usize)(stack_top - 1);
    ins->ebp = ins->esp;
}

void arch_thread_exit() {
    asm("int $0xC2");
}

void NO_RETURN arch_thread_resume(ArchThreadInstance* ins) {
    current_running_thread = ins;

    if (ins->first_run) {
        ins->first_run = false;

        thread_run_entry = ins->entry;
        thread_run_param = (usize)ins->param;
        thread_run_esp   = ins->esp;
        thread_run_ebp   = ins->ebp;

        thread_run();
    } else {
        int_return(ins->esp, ins->ebp);
    }
}

void arch_thread_invoke_exception(Exception* excpt) {
    if (excpt_handler == nullptr)
        panic("No thread exception handler assigned!");

    excpt_handler(excpt);
}

static void arch_thread_invoke_yield(ArchThreadYieldStatus status) {
    if (!yield_handler)
        panic("No yield handler assigned!");

    if (current_running_thread && !arch_cpu_was_idling) {
        current_running_thread->esp = int_get_esp();
        current_running_thread->ebp = int_get_ebp();
    }

    yield_handler(status);
}

void kthread_await(ThreadSignal* signal) {
    asm("int $0xC1\n" : : "a" (signal));
}

void arch_thread_init() {
    int_register_irq(0xC0 - INT_EXCPT_COUNT, [](void*) {
        syscall_handler_func = (void*)syscall_handler;
    });
    
    int_register_irq(0xC1 - INT_EXCPT_COUNT, [](void*) {
        arch_thread_invoke_yield(ArchThreadYieldStatus::AWAIT);
    });
    
    int_register_irq(0xC2 - INT_EXCPT_COUNT, [](void*) {
        arch_thread_invoke_yield(ArchThreadYieldStatus::THREAD_EXIT);
    });

    Timer::add_callback([](u64) {
        arch_thread_invoke_yield(ArchThreadYieldStatus::TIMER);
    });
}