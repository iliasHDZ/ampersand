#pragma once

#include "memory.hpp"

#include "thread.hpp"
#include <data/chain.hpp>
#include <fd/fd.hpp>
#include "credentials.hpp"
#include "error.hpp"

#define PROCESS_STACK_SIZE (128 * KiB)

#define MAX_FD_COUNT 256

class ProcessManager;

struct FileDescriptionHandle {
    bool open = false;
    FileDescription* fd = nullptr;
    usize access_ptr = 0;
    usize perms = 0;

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

    i32 open_handle(FileDescription* fd, usize perms);

private:
    void close();

    void set_pid(usize pid);

    bool is_handle_open(i32 fd);

    SyscallError close_handle(i32 fd);

private:
    Vec<Thread*> threads;

    Vec<FileDescriptionHandle> fd_handles;

    Credentials creds;

    i32 pid = 0;
    ProcessMemory* memory;

    friend class ProcessManager;
};