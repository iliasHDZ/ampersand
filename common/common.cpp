#include "common.hpp"

usize __strlen(const char* str) {
    const char* end = str;
    while (*end)
        end++;
    return end - str;
}

isize __strcmp(const char* str1, const char* str2) {
    while (*str1 && *str2 && *str1 == *str2) {
        str1++;
        str2++;
    }

    return *str1 - *str2;
}

template <typename T>
inline T& access_increment(void*& ptr) {
    return *( ( (T*&)ptr )++ );
}

template <typename T>
inline const T& access_increment(const void*& ptr) {
    return *( ( (T*&)ptr )++ );
}

void* __memcpy(void* dst, const void* src, usize size) {
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

void* __memset(void* dst, int ch, usize size) {
    void* ret = dst;

    usize value = ch;
    value |= value << 8;
    value |= value << 16;
    if (sizeof(usize) == 8)
        value |= value << 32;
    
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

int __memcmp(const void* ptr1, const void* ptr2, usize num) {
    u8* bptr1 = (u8*)ptr1;
    u8* bptr2 = (u8*)ptr2;

    for (usize i = 0; i < num; i++) {
        if (bptr1[i] != bptr2[i])
            return bptr1[i] - bptr2[i];
    }

    return 0;
}