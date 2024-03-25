#pragma once

typedef unsigned char      u8;
typedef unsigned short     u16;
typedef unsigned int       u32;
typedef unsigned long long u64;

typedef char      i8;
typedef short     i16;
typedef int       i32;
typedef long long i64;

#if ARCH_BITS==16
typedef i16 isize;
typedef u16 usize;
#elif ARCH_BITS==32
typedef i32 isize;
typedef u32 usize;
#elif ARCH_BITS==64
typedef i64 isize;
typedef u64 usize;
#endif

typedef u32 time32;
typedef u64 time64;

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

typedef struct {
    usize unused;
} ThreadSignal;

usize strlen(const char* str);

bool streq(const char* str1, const char* str2);

bool memeq(const void* ptr1, const void* ptr2, usize num);

void* memcpy(void* dst, const void* src, usize size);

void* memset(void* dst, int ch, usize size);

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

struct monostate {};

template <typename T>
inline T max(T a, T b) {
    return (a > b) ? a : b;
}

template <typename T>
inline T min(T a, T b) {
    return (a < b) ? a : b;
}

template <typename T>
inline T clamp(T val, T lo, T hi) {
    if (val < lo)
        return lo;
    else if (val > hi)
        return hi;
    else
        return val;
}

template <typename T>
inline T alignceil(T val, T align) {
    T nval = (val >> align) << align;
    if (val != nval)
        nval += 1 << align;

    return nval;
}

template <typename T>
inline T divceil(T num, T div) {
    if (num % div > 0)
        return (num / div) + 1;

    return num / div;
}

template <typename T>
inline void safe_copy(T* dst, T* src, usize count) {
    for (usize i = 0; i < count; i++)
        dst[i] = src[i];
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