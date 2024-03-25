#pragma once

#include "arch.hpp"
#include "exception.hpp"

enum class ArchThreadYieldStatus {
    AWAIT,
    TIMER,
    SYSCALL,
    THREAD_EXIT
};

typedef void(*ArchThreadYieldHandler)(ArchThreadYieldStatus);

typedef void(*ArchThreadSyscallHandler)();

void arch_thread_set_yield_handler(ArchThreadYieldHandler handler);

void arch_thread_set_syscall_handler(ArchThreadSyscallHandler handler);

void arch_thread_set_exception_handler(ExceptionHandler handler);

void arch_thread_get_cpu_state(ArchThreadInstance* ins, CPUState* state);

void arch_thread_create(ArchThreadInstance* ins, void* entry, void* param, void* stack, usize stack_size);

void arch_thread_exit();

void NO_RETURN arch_thread_resume(ArchThreadInstance* ins);