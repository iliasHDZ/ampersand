#pragma once

typedef unsigned char      u8;
typedef unsigned short     u16;
typedef unsigned int       u32;
typedef unsigned long long u64;

typedef char      i8;
typedef short     i16;
typedef int       i32;
typedef long long i64;

// TODO: Make sure it is 64 on 64-bit architectures
typedef i32 isize;
typedef u32 usize;

typedef u32 time32;
typedef u64 time64;

usize __strlen(const char* str);

isize __strcmp(const char* str1, const char* str2);

void* __memcpy(void* dst, const void* src, usize size);

void* __memset(void* dst, int ch, usize size);

int __memcmp(const void* ptr1, const void* ptr2, usize num);

void* __malloc(usize size);

void* __realloc(void* ptr, usize size);

void __free(void *ptr);

bool __is_alloc_available();

struct ThreadSignal {
    usize unused;
};

void __thread_await(ThreadSignal* signal);

void __thread_emit(ThreadSignal* signal);

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