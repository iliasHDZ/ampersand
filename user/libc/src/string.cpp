#include <string.h>

size_t strlen(const char* str) {
    size_t ret = 0;
    while (*str) {
        str++;
        ret++;
    }

    return ret;
}


template <typename T>
inline T& access_increment(void*& ptr) {
    return *( ( (T*&)ptr )++ );
}

template <typename T>
inline const T& access_increment(const void*& ptr) {
    return *( ( (T*&)ptr )++ );
}

void* memcpy(void* dst, const void* src, size_t size) {
    void* ret = dst;
    
    unsigned long long_count = size / sizeof(unsigned long);
    while (long_count) {
        access_increment<unsigned long>(dst) = access_increment<unsigned long>(src);
        long_count--;
    }

    while (size % sizeof(unsigned long)) {
        access_increment<unsigned char>(dst) = access_increment<unsigned char>(src);
        size--;
    }

    return ret;
}

void* memset(void* s, int c, size_t n) {
    unsigned char v = c;
    char* b = (char*)s;

    while (n) {
        *b = v;
        b++;
        n--;
    }

    return s;
}