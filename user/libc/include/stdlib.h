#ifndef STDLIB_H
#define STDLIB_H

#define EXIT_SUCCESS 0
#define EXIT_FAILURE -1

typedef unsigned long size_t;

#ifdef __cplusplus
extern "C" {
#endif

void exit(int status);

void* malloc(size_t size);
void* realloc(void* ptr, size_t size);
void free(void *ptr);

#define _Exit exit
#define _exit exit

#ifdef __cplusplus
}
#endif

#endif // STDLIB_H