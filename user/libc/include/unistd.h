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
#define SYSCALL_POLL  0x09
#define SYSCALL_EXEC  0x0a

#define SYSCALL_IOCTL 0x10
#define SYSCALL_EXIT  0xff

#define SYSCALL_BRK         0x1000
#define SYSCALL_SBRK        0x1001
#define SYSCALL_SAVELIST    0x1002
#define SYSCALL_RESTORELIST 0x1003

#define __LIST_ARGV 0
#define __LIST_ENVP 1

#define SEEK_SET 0
#define SEEK_CUR 1
#define SEEK_END 2

#define STDIN_FILENO  0
#define STDOUT_FILENO 1
#define STDERR_FILENO 2

#define MAX_EXECL_ARGS 64

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

extern char** environ;
int execl(const char* path, ...);
int execle(const char* path, ...);
int execv(const char* path, char* const argv[]);
int execve(const char* path, char* const argv[], char* const envp[]);

#ifdef __cplusplus
}
#endif

#endif // UNISTD_H