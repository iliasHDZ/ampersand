#pragma once

#include "memory.hpp"

#include "thread.hpp"
#include <data/chain.hpp>
#include <fd/fd.hpp>
#include "credentials.hpp"
#include "error.hpp"

#define PROCESS_STACK_SIZE (128 * KiB)

class ProcessManager;

struct FileDescriptionHandle {
    bool open = false;
    FileDescription* fd = nullptr;
    usize access_ptr = 0;
    usize perms = 0;

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
    i32 sys_ioctl(i32 fd, i32 request, usize* args);

private:
    void close();

    void set_pid(usize pid);

    bool is_valid_handle(i32 fd);

    bool is_handle_open(i32 fd);

    i32 alloc_handle();

    i32 open_handle(FileDescription* fd, usize perms);

    SyscallError close_handle(i32 fd);

    i32 duplicate_handle(i32 src, i32 dst = -1);

private:
    Vec<Thread*> threads;

    Vec<FileDescriptionHandle> fd_handles;

    Credentials creds;

    i32 pid = 0;
    ProcessMemory* memory;

    friend class ProcessManager;
};