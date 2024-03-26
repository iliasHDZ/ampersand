#include "process.hpp"
#include <fd/manager.hpp>
#include <fs/fs_manager.hpp>
#include <fcntl.h>
#include "elf.hpp"

usize FileDescriptionHandle::read(void* dst, usize size) {
    usize ret = fd->read(dst, access_ptr, size);
    if (fd->has_size())
        access_ptr += ret;

    // TODO: Blocking until everything is read

    return ret;
}

usize FileDescriptionHandle::write(void* src, usize size) {
    usize ret = fd->write(src, access_ptr, size);
    if (fd->has_size())
        access_ptr += ret;

    // TODO: Blocking until everything is written

    return ret;
}

Process::Process()
    : creds(0, 0)
{
    memory = new ProcessMemory();
}

Process::~Process() {
    delete memory;
}

SyscallError Process::exec(FileDescription* file) {
    if (!file->may_exec)
        return EACCES;

    ProcessMemory* mem = new ProcessMemory();

    ExecutableInfo info = Elf::load_executable(file, mem);

    if (!info.success) {
        delete mem;
        return ENOEXEC;
    }

    u64 stack_pages = PROCESS_STACK_SIZE / ARCH_PAGE_SIZE;

    MemorySegment* mseg = MemorySegment::create(stack_pages, VMEM_PAGE_READ | VMEM_PAGE_WRITE);

    if (mem->map_segment(mseg, PageRange::base_limit(0xC0000 - stack_pages, 0xC0000)) == nullptr) {
        delete mem;
        delete mseg;
        return ENOMEM;
    }

    void* stack = (void*)((0xC0000 - stack_pages) * ARCH_PAGE_SIZE);

    Thread* thread = ThreadScheduler::get()->create_user_thread((ThreadEntry)info.entry, this, mem->get_vmem(), 0, stack, stack_pages * ARCH_PAGE_SIZE);

    Vec<Thread*> thrds;

    thrds.append(thread);

    bool current_in_threads = false;

    for (auto thrd : threads) {
        if (thrd != ThreadScheduler::get()->current())
            ThreadScheduler::get()->exit(thrd);
        else
            current_in_threads = true;
    }

    threads = thrds;
    delete memory;
    memory = mem;

    if (current_in_threads)
        kthread_exit();

    return ENOERR;
}

void Process::close() {
    for (usize i = 0; i < fd_handles.size(); i++) {
        if (fd_handles[i].open)
            close_handle(i);
    }

    for (auto thread : threads)
        ThreadScheduler::get()->exit(thread);
}

void Process::set_pid(usize pid) {
    this->pid = pid;
}

i32 Process::open_handle(FileDescription* fd, usize perms) {
    for (usize i = 0; i < fd_handles.size(); i++) {
        if (!fd_handles[i].open) {
            FileDescriptionHandle* fdh = &fd_handles[i];

            fdh->open = true;
            fdh->fd = fd;
            fdh->access_ptr = 0;

            return i;
        }
    }

    if (fd_handles.size() >= MAX_FD_COUNT)
        return -EMFILE;

    i32 ret = fd_handles.size();
    fd_handles.append({ true, fd, 0 });

    return ret;
}

bool Process::is_handle_open(i32 fd) {
    if (fd_handles.size() <= fd)
        return false;

    return fd_handles[fd].open;
}

SyscallError Process::close_handle(i32 fd) {
    FileDescriptionHandle* fdh = &fd_handles[fd];
    if (!fdh->open)
        return EBADF;
    
    SyscallError err = FileDescriptionManager::get()->close(fdh->fd);

    fdh->open = false;
    fdh->fd   = nullptr;
    return err;
}