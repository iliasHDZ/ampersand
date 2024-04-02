#ifndef STDIO_H
#define STDIO_H

#include <stdarg.h>
#include <printf.h>

typedef unsigned long size_t;
typedef long ssize_t;
typedef long off_t;

#define SEEK_SET 0
#define SEEK_CUR 1
#define SEEK_END 2

#ifdef __cplusplus
extern "C" {
#endif

int putchar(int ch);

int puts(const char* str);

#ifdef __cplusplus
}
#endif

#endif // STDIO_H