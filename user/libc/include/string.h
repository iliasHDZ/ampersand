#ifndef STRING_H
#define STRING_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

size_t strlen(const char *);

void* memcpy(void* s1, const void* s2, size_t n);

#ifdef __cplusplus
}
#endif

#endif // STRING_H