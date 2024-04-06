#include "process.hpp"
#include <fd/manager.hpp>
#include <fs/fs_manager.hpp>
#include <logger.hpp>
#include <fcntl.h>
#include <limits.h>
#include "elf.hpp"

ThreadSignal Process::process_exit;
int          Process::process_exit_pid = 0;
int          Process::process_exit_status = 0;

usize FileDescriptionHandle::read_raw(void* dst, usize size) {
    MutexLock _(fd->access_mutex);

    usize ret = fd->read(dst, access_ptr, size);
    if (fd->has_size())
        access_ptr += ret;
    return ret;
}

usize FileDescriptionHandle::write_raw(void* src, usize size) {
    MutexLock _(fd->access_mutex);

    usize ret = fd->write(src, access_ptr, size);
    if (fd->has_size())
        access_ptr += ret;
    return ret;
}

usize FileDescriptionHandle::read(void* dst, usize size) {
    u8* buf = (u8*)dst;

    usize ret = read_raw(buf, size);

    if (!fd->should_block() || (flags & O_NONBLOCK))
        return ret;

    while (ret < size) {
        FileDescription::await_event();
        if (!fd->can_read())
            continue;
        
        ret += read_raw(buf + ret, size - ret);
    }

    return ret;
}

usize FileDescriptionHandle::write(void* src, usize size) {
    u8* buf = (u8*)src;

    if (flags & O_APPEND)
        access_ptr = fd->get_size();

    usize ret = write_raw(buf, size);

    if (!fd->should_block() || (flags & O_NONBLOCK))
        return ret;

    while (ret < size) {
        FileDescription::await_event();
        if (!fd->can_write())
            continue;
        
        ret += write_raw(buf + ret, size - ret);
    }

    return ret;
}

Process::Process()
    : creds(0, 0)
{
    memory = new ProcessMemory();
    cwd = "/";
}

Process::~Process() {
    delete memory;

    for (isize i = 0; i >= argv.size(); i++)
        delete[] argv[i];

    for (isize i = 0; i >= envp.size(); i++)
        delete[] envp[i];
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

    usize heap_base_page = 0x400;

    for (auto& seg : mem->get_segments())
        heap_base_page = max((u64)heap_base_page, seg->get_pagerange().limit);

    brk = (void*)((heap_base_page + 1) * ARCH_PAGE_SIZE);

    MemorySegment* heap_seg = MemorySegment::create(1, VMEM_PAGE_READ | VMEM_PAGE_WRITE);
    
    mem->heap = mem->map_segment(heap_seg, PageRange::base_limit(heap_base_page, heap_base_page + 1));
    if (mem->heap == nullptr) {
        delete mem;
        delete heap_seg;
        return ENOMEM;
    }

    u64 stack_pages = PROCESS_STACK_SIZE / ARCH_PAGE_SIZE;

    MemorySegment* mseg = MemorySegment::create(stack_pages, VMEM_PAGE_READ | VMEM_PAGE_WRITE, true);

    if (mem->map_segment(mseg, PageRange::base_limit(0xC0000 - stack_pages, 0xC0000)) == nullptr) {
        delete mem;
        delete heap_seg;
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

    thread->set_running(true);

    if (current_in_threads)
        kthread_exit();

    return ENOERR;
}

void Process::close(i32 status) {
    for (usize i = 0; i < fd_handles.size(); i++) {
        if (fd_handles[i].open)
            close_handle(i);
    }

    for (auto thread : threads)
        ThreadScheduler::get()->exit(thread);

    process_exit_pid = pid;
    process_exit_status = status;
    kthread_emit(&process_exit);
}

void Process::set_pid(usize pid) {
    this->pid = pid;
}

bool Process::is_valid_handle(i32 fd) {
    return fd >= 0 && fd < OPEN_MAX;
}

bool Process::is_handle_open(i32 fd) {
    if (fd < 0)
        return false;

    if (fd_handles.size() <= fd)
        return false;

    return fd_handles[fd].open;
}

i32 Process::open_handle(FileDescription* fd, usize perms) {
    for (usize i = 0; i < fd_handles.size(); i++) {
        if (!fd_handles[i].open) {
            FileDescriptionHandle* fdh = &fd_handles[i];

            fdh->open = true;
            fdh->fd = fd;
            fdh->access_ptr = 0;

            FileDescriptionManager::get()->open(fd);
            return i;
        }
    }

    if (fd_handles.size() >= OPEN_MAX)
        return -EMFILE;

    i32 ret = fd_handles.size();
    fd_handles.append(FileDescriptionHandle(fd, perms));

    FileDescriptionManager::get()->open(fd);
    return ret;
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

i32 Process::duplicate_handle(i32 src, i32 dst) {
    if (!is_handle_open(src))
        return -EBADF;

    if (dst != -1 && !is_valid_handle(dst))
        return -EBADF;

    if (dst != -1 && is_handle_open(dst)) {
        if (close_handle(dst) != ENOERR)
            return -EIO;
    
        fd_handles[dst] = fd_handles[src];
        FileDescriptionManager::get()->open(fd_handles[src].fd);
        return dst;
    }

    FileDescriptionHandle* fdh_src = &fd_handles[src];

    dst = open_handle(fdh_src->fd, fdh_src->flags);
    if (dst < 0)
        return dst;

    fd_handles[dst] = fd_handles[src];
    return dst;
}

Process* Process::fork(Thread* caller) {
    Process* dst = new Process();

    dst->creds  = creds;
    dst->memory = memory->fork();

    dst->brk = brk;
    dst->cwd = cwd;

    for (int i = 0; i < fd_handles.size(); i++) {
        if (!fd_handles[i].open) {
            dst->fd_handles.append(FileDescriptionHandle());
            continue;
        }

        FileDescriptionHandle* fdh_src = &fd_handles[i];

        FileDescriptionManager::get()->open(fdh_src->fd);
        dst->fd_handles.append(FileDescriptionHandle(fdh_src->fd, fdh_src->flags, fdh_src->access_ptr));
    }
    
    Thread* nthread = ThreadScheduler::get()->fork(caller, dst->memory->get_vmem(), dst);
    dst->threads.append(nthread);
    nthread->set_running(true);
    return dst;
}

Path Process::resolve(const char* path) {
    return cwd.resolve(path);
}

void Process::set_brk(void* nbrk) {
    usize nbrk_page = (usize)nbrk / ARCH_PAGE_SIZE;
    if ((usize)nbrk % ARCH_PAGE_SIZE)
        nbrk_page++;

    nbrk_page = max((u64)nbrk_page, memory->heap->get_pagerange().base);
    usize count = nbrk_page - memory->heap->get_pagerange().base;

    memory->heap->get_mem_segment()->set_page_count(count);
    memory->heap->resize(count);
}

i32 Process::sys_savelist(bool env, const char** list) {
    Vec<char*>& dst = env ? envp : argv;

    for (isize i = dst.size() - 1; i >= 0; i--){
        delete[] dst[i];
        dst.remove(i);
    }
    
    while (*list) {
        const char* str = *list;
        usize len = strlen(str);

        char* strdst = new char[len + 1];
        memcpy(strdst, str, len + 1);

        dst.append(strdst);
        list++;
    }

    return 0;
}

i32 Process::sys_restorelist(bool env, char* dst) {
    Vec<char*>& src = env ? envp : argv;

    if (dst == nullptr) {
        i32 len = 0;

        for (auto str : src)
            len += strlen(str) + 1;

        return len;
    }

    for (auto str : src) {
        usize len = strlen(str);

        memcpy(dst, str, len + 1);
        dst += len + 1;
    }

    for (isize i = src.size() - 1; i >= 0; i--){
        delete[] src[i];
        src.remove(i);
    }

    return 0;
}