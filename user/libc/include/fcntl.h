#ifndef FCNTL_H
#define FCNTL_H

#include "sys/stat.h"
#include "unistd.h"

#define O_RDONLY   0b00000001
#define O_WRONLY   0b00000010
#define O_RDWR     0b00000100
#define O_APPEND   0b00001000
#define O_CREAT    0b00010000
#define O_NONBLOCK 0b00100000
#define O_APPEND   0b01000000

#ifdef __cplusplus
extern "C" {
#endif

// Variadic arguments not yet supported
int open(const char* path, int oflags);

#ifdef __cplusplus
}
#endif

#endif // FCNTL_H