#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <stropts.h>
#include <stdarg.h>
#include <poll.h>
#include <sys/wait.h>
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

int savelist(int type, const char** list) {
    int ret = _kernel_syscall(SYSCALL_SAVELIST, type, (u32)list, 0);
    return _set_errno(ret) ? -1 : ret;
}

int restorelist(int type, char* list) {
    int ret = _kernel_syscall(SYSCALL_RESTORELIST, type, (u32)list, 0);
    return _set_errno(ret) ? -1 : ret;
}

static char** remakelist(int type) {
    size_t size = restorelist(type, nullptr);

    if (size == 0) {
        char** ret = (char**)malloc(sizeof(char*));
        *ret = 0;
        return ret;
    }

    char* strtab = (char*)malloc(size);

    restorelist(type, strtab);

    char prevc = 0;
    int count = 0;
    for (int i = 0; i < size; i++) {
        if (prevc == 0 && strtab[i])
            count++;
        prevc = strtab[i];
    }

    char** strlist = (char**)malloc(sizeof(char*) * (count + 1));
    strlist[count] = 0;

    prevc = 0;
    int idx = 0;
    for (int i = 0; i < size; i++) {
        if (prevc == 0 && strtab[i])
            strlist[idx++] = &strtab[i];
        prevc = strtab[i];
    }

    return strlist;
}

extern "C" int    __main_argc = 0;
extern "C" char** __main_argv = nullptr;

char** environ = nullptr;

static bool malloc_heap = true;

#ifndef __INTELLISENSE__
void* operator new(long unsigned int size) {
    if (malloc_heap)
        return (void*)((char*)sbrk(0) - sizeof(LCHeap));

    return malloc(size);
}

void operator delete(void* ptr) {
    free(ptr);
}

void operator delete(void* ptr, long unsigned int size) {
    free(ptr);
}

void* operator new[](long unsigned int size) {
    return malloc(size);
}

void operator delete[](void* ptr) {
    free(ptr);
}
#endif

extern "C" void _init_libc() {
    heap = new LCHeap(sbrk(0), sbrk(0));

    malloc_heap = false;

    heap->set_resize_func([](void* limit) -> bool {
        brk(limit);
        return true;
    });

    __main_argv = remakelist(__LIST_ARGV);
    __main_argc = 0;
    while (__main_argv[__main_argc])
        __main_argc++;
    
    environ = remakelist(__LIST_ENVP);
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

static int exec(const char* path) {
    int ret = _kernel_syscall(SYSCALL_EXEC, (u32)path, 0, 0);
    return _set_errno(ret) ? -1 : ret;
}

int execl(const char* path, ...) {
    va_list vargs;
    va_start(vargs, path);

    const char* args[MAX_EXECL_ARGS + 1] = { 0 };

    int count = 0;
    for (; count < MAX_EXECL_ARGS; count++) {
        const char* arg = va_arg(vargs, const char*);
        if (arg == 0)
            break;
        args[count] = arg;
    }

    args[count] = 0;

    execv(path, (char* const*)args);

    va_end(vargs);
}

int execle(const char* path, ...) {
    va_list vargs;
    va_start(vargs, path);

    const char* args[MAX_EXECL_ARGS + 1] = { 0 };

    int count = 0;
    for (; count < MAX_EXECL_ARGS; count++) {
        const char* arg = va_arg(vargs, const char*);
        if (arg == 0)
            break;
        args[count] = arg;
    }

    const char* envp = va_arg(vargs, const char*);

    execve(path, (char* const*)args, (char* const*)envp);

    va_end(vargs);
}

int execv(const char* path, char* const argv[]) {
    savelist(__LIST_ARGV, (const char**)argv);
    savelist(__LIST_ENVP, (const char**)environ);
    exec(path);
}

int execve(const char* path, char* const argv[], char* const envp[]) {
    savelist(__LIST_ARGV, (const char**)argv);
    savelist(__LIST_ENVP, (const char**)envp);
    exec(path);
}

pid_t waitpid(pid_t pid, int* stat_loc, int options) {
    int ret = _kernel_syscall(SYSCALL_WAITPID, pid, (u32)stat_loc, options);
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