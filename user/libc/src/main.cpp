#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <stropts.h>
#include <stdarg.h>
#include <poll.h>
#include "common.hpp"
#include "heap.hpp"

int errno = 0;

static bool _set_errno(ssize_t value) {
    if (value < 0) {
        errno = -value;
        return true;
    } else
        return false;
}

int brk(void* brk) {
    ssize_t ret = _kernel_syscall(SYSCALL_BRK, (u32)brk, 0, 0);
    return _set_errno(ret) ? -1 : ret;
}

void* sbrk(size_t brk) {
    return (void*)_kernel_syscall(SYSCALL_SBRK, brk, 0, 0);
}

LCHeap* heap = nullptr;

extern "C" void _init_libc() {
    heap = ((LCHeap*)sbrk(0)) - 1;
    *heap = LCHeap(sbrk(0), sbrk(0));

    heap->set_resize_func([](void* limit) -> bool {
        brk(limit);
        return true;
    });
}

void exit(int status) {
    _kernel_syscall(SYSCALL_EXIT, status, 0, 0);
    for (;;);
}

void* malloc(size_t size) {
    if (heap == nullptr)
        return nullptr;

    return heap->malloc(size);
}

void* realloc(void* ptr, size_t size) {
    if (heap == nullptr)
        return nullptr;

    return heap->realloc(ptr, size);
}

void free(void *ptr) {
    if (heap == nullptr)
        return;

    heap->free(ptr);
}

int close(int fd) {
    return _set_errno(_kernel_syscall(SYSCALL_CLOSE, fd, 0, 0)) ? -1 : 0;
}

ssize_t read(int fildes, void* buf, size_t nbyte) {
    ssize_t ret = _kernel_syscall(SYSCALL_READ, fildes, (u32)buf, nbyte);
    return _set_errno(ret) ? -1 : ret;
}

ssize_t write(int fildes, const void* buf, size_t nbyte) {
    ssize_t ret = _kernel_syscall(SYSCALL_WRITE, fildes, (u32)buf, nbyte);
    return _set_errno(ret) ? -1 : ret;
}

off_t lseek(int fildes, off_t offset, int whence) {
    off_t ret = _kernel_syscall(SYSCALL_LSEEK, fildes, offset, whence);
    return _set_errno(ret) ? -1 : ret;
}

int pipe(int fildes[2]) {
    int ret = _kernel_syscall(SYSCALL_PIPE, (u32)fildes, 0, 0);
    return _set_errno(ret) ? -1 : ret;
}

int dup(int fildes) {
    int ret = _kernel_syscall(SYSCALL_DUP, fildes, 0, 0);
    return _set_errno(ret) ? -1 : ret;
}

int dup2(int fildes, int fildes2) {
    int ret = _kernel_syscall(SYSCALL_DUP2, fildes, fildes2, 0);
    return _set_errno(ret) ? -1 : ret;
}

int open(const char* path, int oflags) {
    int ret = _kernel_syscall(SYSCALL_OPEN, (u32)path, oflags, 0);
    return _set_errno(ret) ? -1 : ret;
}

pid_t fork() {
    int ret = _kernel_syscall(SYSCALL_FORK, 0, 0, 0);
    return _set_errno(ret) ? -1 : ret;
}

int poll(struct pollfd fds[], nfds_t nfds, int timeout) {
    int ret = _kernel_syscall(SYSCALL_POLL, (u32)fds, nfds, timeout);
    return _set_errno(ret) ? -1 : ret;
}

int exec(const char* path) {
    int ret = _kernel_syscall(SYSCALL_EXEC, (u32)path, 0, 0);
    return _set_errno(ret) ? -1 : ret;
}

int ioctl(int fd, int request, ...) {
    va_list args;
    va_start(args, request);

    int ioctl_args[16] = { 0 };

    int count = _kernel_syscall(SYSCALL_IOCTL, fd, request, 0);

    for (int i = 0; i < count; i++)
        ioctl_args[i] = va_arg(args, int);

    va_end(args);

    int ret = _kernel_syscall(SYSCALL_IOCTL, fd, request, (u32)ioctl_args);
    return _set_errno(ret) ? -1 : ret;
}

#ifndef __INTELLISENSE__
void* operator new(long unsigned int size) {
    return nullptr;
}

void operator delete(void* ptr) {
    
}

void operator delete(void* ptr, long unsigned int size) {
    
}

void* operator new[](long unsigned int size) {
    return nullptr;
}

void operator delete[](void* ptr) {
    
}
#endif