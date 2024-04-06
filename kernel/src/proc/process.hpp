#pragma once

#include "memory.hpp"

#include "thread.hpp"
#include <common/chain.hpp>
#include <fd/fd.hpp>
#include "credentials.hpp"
#include "error.hpp"
#include <common/path.hpp>
#include <poll.h>
#include <fcntl.h>

#define PROCESS_STACK_SIZE (128 * KiB)

class ProcessManager;

struct FileDescriptionHandle {
    bool open = false;
    FileDescription* fd = nullptr;
    usize access_ptr = 0;
    usize flags = 0;

    inline FileDescriptionHandle() {
        open = false;
    }

    inline FileDescriptionHandle(FileDescription* fd, usize flags, usize access_ptr = 0) {
        open = true;
        this->fd = fd;
        this->flags = flags;
        this->access_ptr = access_ptr;
    }

    inline bool may_read()  { return (flags & O_RDONLY) || (flags & O_RDWR); };
    inline bool may_write() { return (flags & O_WRONLY) || (flags & O_RDWR); };

    usize read_raw(void* dst, usize size);

    usize write_raw(void* src, usize size);

    usize read(void* dst, usize size);

    usize write(void* src, usize size);
};

class Process : public DLChainItem {
public:
    Process();

    ~Process();

    inline usize get_pid() { return pid; };

    inline ProcessMemory* get_memory() { return memory; };

    SyscallError exec(FileDescription* file);

    // ALL SYSCALLS HANDLERS (defined in syscall.cpp)
    isize sys_read(i32 fd, void* buf, usize size);
    isize sys_write(i32 fd, void* buf, usize size);
    i32 sys_open(const char* path, isize oflags);
    i32 sys_close(i32 fd);
    isize sys_lseek(i32 fd, isize offset, u32 whence);
    i32 sys_pipe(i32* out);
    i32 sys_dup(i32 src);
    i32 sys_dup2(i32 src, i32 dst);
    i32 sys_poll(struct pollfd* fds, i32 nfds, i32 timeout);
    i32 sys_exec(const char* path);
    i32 sys_waitpid(i32 pid, i32* stat_loc, int options);
    i32 sys_ioctl(i32 fd, i32 request, usize* args);

private:
    void close(i32 status);

    void set_pid(usize pid);

    bool is_valid_handle(i32 fd);

    bool is_handle_open(i32 fd);

    i32 open_handle(FileDescription* fd, usize perms);

    SyscallError close_handle(i32 fd);

    i32 duplicate_handle(i32 src, i32 dst = -1);

    Process* fork(Thread* caller);

    Path resolve(const char* path);

    void set_brk(void* nbrk);

    i32 sys_savelist(bool env, const char** list);
    i32 sys_restorelist(bool env, char* dst);

private:
    static ThreadSignal process_exit;
    static int          process_exit_pid;
    static int          process_exit_status;

private:
    Vec<Thread*> threads;

    Vec<FileDescriptionHandle> fd_handles;

    Vec<char*> argv;
    Vec<char*> envp;

    Credentials creds;

    void* brk;

    Path cwd;

    i32 pid = 0;
    ProcessMemory* memory;

    friend class ProcessManager;
};