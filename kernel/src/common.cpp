#include <common.h>
#include <logger.hpp>

#include "mem/manager.hpp"
#include "mem/paging.hpp"
#include "proc/thread.hpp"

extern "C" void* kmalloc(usize size) {
    return MemoryManager::get()->kmalloc(size);
}

extern "C" void* kmalloc_align(usize size, u8 align) {
    return MemoryManager::get()->kmalloc(size, align);
}

extern "C" void kfree(void* ptr) {
    MemoryManager::get()->kfree(ptr);
}

extern "C" void* krealloc(void* ptr, usize size) {
    return MemoryManager::get()->krealloc(ptr, size);
}

extern "C" bool is_alloc_available() {
    return MemoryManager::get()->has_init();
}

extern "C" void kthread_create(ThreadFunc func, void* param) {
    ThreadScheduler::get()->create_thread(func, param);
}

extern "C" void kthread_emit(ThreadSignal* signal) {
    ThreadScheduler::get()->emit(signal);
}

void* __malloc(usize size) {
    return kmalloc(size);
}

void* __realloc(void* ptr, usize size) {
    return krealloc(ptr, size);
}

void __free(void *ptr) {
    return kfree(ptr);
}

bool __is_alloc_available() {
    return is_alloc_available();
}

void __thread_await(ThreadSignal* signal) {
    return kthread_await(signal);
}

void __thread_emit(ThreadSignal* signal) {
    return kthread_emit(signal);
}

extern "C" void kthread_exit() {
    arch_thread_exit();
}

extern "C" void* vir_addr_to_phy(void* addr) {
    return (void*)VirtualMemoryManager::get()->get_current()->vir_addr_to_phy((u64)(u32)addr);
}

extern "C" time64 time() {
    return 1709999539;
}