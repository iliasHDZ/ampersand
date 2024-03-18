#include <common.h>
#include <logger.hpp>

#include "mem/manager.hpp"
#include "thread.hpp"

extern "C" usize strlen(const char* str) {
    const char* begin = str;
    while (*str) str++;

    return str - begin;
}

extern "C" bool streq(const char* str1, const char* str2) {
    while (*str1 && *str2 && *str1 == *str2) {
        str1++;
        str2++;
    }

    return *str1 == *str2;
}

template <typename T>
inline T& access_increment(void*& ptr) {
    return *( ( (T*&)ptr )++ );
}

template <typename T>
inline const T& access_increment(const void*& ptr) {
    return *( ( (T*&)ptr )++ );
}

extern "C" void* memcpy(void* dst, const void* src, usize size) {
    void* ret = dst;
    
    usize usize_count = size / sizeof(usize);
    while (usize_count) {
        access_increment<usize>(dst) = access_increment<usize>(src);
        usize_count--;
    }

    while (size % sizeof(usize)) {
        access_increment<u8>(dst) = access_increment<u8>(src);
        size--;
    }

    return ret;
}

extern "C" void* memset(void* dst, int ch, usize size) {
    void* ret = dst;

    usize value = ch;
    value |= value << 8;
    value |= value << 16;
#if ARCH_BITS==64
    value |= value << 32;
#endif
    
    usize usize_count = size / sizeof(usize);
    while (usize_count) {
        access_increment<usize>(dst) = value;
        usize_count--;
    }

    while (size % sizeof(usize)) {
        access_increment<u8>(dst) = value & 0xff;
        size--;
    }

    return ret;
}

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

extern "C" void panic(const char* err) {
    Log::ERR() << "KERNEL PANIC!\n";
    Log::ERR() << '\n';
    Log::ERR() << "Error: " << err << '\n';
    Log::ERR() << '\n';
    Log::ERR() << "Ampersand & v" << AMPERSAND_VERSION << '\n';
    Log::ERR() << '\n';
    Log::ERR() << "Please report this error to the developer\n";
    Log::ERR() << "You can manually restart or shutdown your computer\n";
    Log::ERR() << '\n';

    arch_lock_cpu();
}

extern "C" void kthread_create(ThreadFunc func, void* param) {
    ThreadScheduler::get()->create_thread(func, param);
}

extern "C" void kthread_emit(ThreadSignal* signal) {
    ThreadScheduler::get()->emit(signal);
}

extern "C" time64 time() {
    return 1709999539;
}