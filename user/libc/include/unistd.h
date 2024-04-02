#ifndef UNISTD_H
#define UNISTD_H

#define _POSIX_VERSION 200809L
#define _XOPEN_VERSION 700

#define SYSCALL_READ  0x00
#define SYSCALL_WRITE 0x01
#define SYSCALL_OPEN  0x02
#define SYSCALL_CLOSE 0x03
#define SYSCALL_LSEEK 0x04
#define SYSCALL_PIPE  0x05
#define SYSCALL_DUP   0x06
#define SYSCALL_DUP2  0x07
#define SYSCALL_FORK  0x08
#define SYSCALL_EXEC  0x09

#define SYSCALL_IOCTL 0x10
#define SYSCALL_EXIT  0xff

#define SEEK_SET 0
#define SEEK_CUR 1
#define SEEK_END 2

#define STDIN_FILENO  0
#define STDOUT_FILENO 1
#define STDERR_FILENO 2

#ifdef __cplusplus
extern "C" {
#endif

typedef unsigned long size_t;
typedef long ssize_t;
typedef unsigned int uid_t;
typedef unsigned int gid_t;
typedef long off_t;
typedef unsigned int pid_t;

int close(int fd);

ssize_t read(int fildes, void* buf, size_t nbyte);
ssize_t write(int fildes, const void* buf, size_t nbyte);
off_t lseek(int fildes, off_t offset, int whence);

int pipe(int fildes[2]);

int dup(int fildes);
int dup2(int fildes, int fildes2);

pid_t fork(void);

int exec(const char* path);

#ifdef __cplusplus
}
#endif

#endif // UNISTD_H