#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include "common.hpp"

int errno = 0;

static bool _set_errno(ssize_t value) {
    if (value < 0) {
        errno = -value;
        return true;
    } else
        return false;
}

u32 _kernel_syscall(u32 a, u32 b, u32 c, u32 d) {
    asm("int $0xC0" : "=a" (a) : "a" (a), "b" (b), "c" (c), "d" (d));
    return a;
}

void exit(int status) {
    _kernel_syscall(SYSCALL_EXIT, status, 0, 0);
    for (;;);
}

ssize_t read(int fildes, void* buf, size_t nbyte) {
    ssize_t ret = _kernel_syscall(SYSCALL_READ, fildes, (u32)buf, nbyte);
    return _set_errno(ret) ? -1 : ret;
}

ssize_t write(int fildes, const void* buf, size_t nbyte) {
    ssize_t ret = _kernel_syscall(SYSCALL_WRITE, fildes, (u32)buf, nbyte);
    return _set_errno(ret) ? -1 : ret;
}

int close(int fd) {
    return _set_errno(_kernel_syscall(SYSCALL_CLOSE, fd, 0, 0)) ? -1 : 0;
}

int open(const char* path, int oflags) {
    int ret = _kernel_syscall(SYSCALL_OPEN, (u32)path, oflags, 0);
    return _set_errno(ret) ? -1 : ret;
}