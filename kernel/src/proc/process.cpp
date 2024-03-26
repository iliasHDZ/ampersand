#include "process.hpp"
#include "elf.hpp"

Process::Process() {
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
    for (auto thread : threads)
        ThreadScheduler::get()->exit(thread);
}

void Process::set_pid(usize pid) {
    this->pid = pid;
}