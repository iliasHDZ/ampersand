#pragma once

#include <common/common.hpp>

#define KiB (1024)
#define MiB (1024 * 1024)
#define GiB (1024 * 1024 * 1024)
#define TiB (1024 * 1024 * 1024 * 1024)

#define PACKED_STRUCT __attribute__((__packed__))

#define NO_RETURN

#ifdef __cplusplus
extern "C" {
#endif

typedef void(*ThreadFunc)(void*);

#define strlen __strlen
#define strcmp __strcmp

#define memcpy __memcpy
#define memset __memset
#define memcmp __memcmp

inline bool streq(const char* ptr1, const char* ptr2) {
    return strcmp(ptr1, ptr2) == 0;
}

inline bool memeq(const void* ptr1, const void* ptr2, usize size) {
    return memcmp(ptr1, ptr2, size) == 0;
}

void* kmalloc(usize size);

void* kmalloc_align(usize size, u8 align);

void kfree(void* ptr);

void* krealloc(void* ptr, usize size);

bool is_alloc_available();

void panic(const char* err);

void kthread_create(ThreadFunc func, void* param);

void kthread_await(ThreadSignal* signal);

void kthread_emit(ThreadSignal* signal);

void kthread_exit();

void* vir_addr_to_phy(void* addr);

time64 time();

static inline char get_hex_char(u8 val) {
    if (val >= 0xa)
        return 'a' + (val - 0xa);
    
    return '0' + val;
}

#ifdef __cplusplus
}

#ifndef __INTELLISENSE__
inline void* operator new(long unsigned int size) {
    return kmalloc(size);
}

inline void operator delete(void* ptr) {
    kfree(ptr);
}

inline void operator delete(void* ptr, long unsigned int size) {
    kfree(ptr);
}

inline void* operator new[](long unsigned int size) {
    return kmalloc(size);
}

inline void operator delete[](void* ptr) {
    kfree(ptr);
}
#endif

#endif