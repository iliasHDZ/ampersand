#ifndef UNISTD_H
#define UNISTD_H

#define _POSIX_VERSION 200809L
#define _XOPEN_VERSION 700

#define SYSCALL_READ 0
#define SYSCALL_WRITE 1
#define SYSCALL_OPEN 2
#define SYSCALL_CLOSE 3
#define SYSCALL_EXIT 0xff

typedef unsigned int size_t;
typedef int ssize_t;

#ifdef __cplusplus
extern "C" {
#endif

int close(int fd);

ssize_t read(int fildes, void* buf, size_t nbyte);

ssize_t write(int fildes, const void* buf, size_t nbyte);

#ifdef __cplusplus
}
#endif

#endif // UNISTD_H