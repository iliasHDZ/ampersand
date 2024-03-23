#include "drv/vga_text.hpp"

#include "boot/boot.hpp"
#include "../test/tests.hpp"
#include "mem/memory.hpp"
#include "mem/manager.hpp"
#include "mem/paging.hpp"
#include "drv/pci/pci.hpp"
#include "proc/thread.hpp"
#include "fs/fs_manager.hpp"
#include "proc/manager.hpp"
#include "fs/devfs.hpp"

#include "drv/debugout.hpp"

#include <arch/arch.hpp>

#include <logger.hpp>

extern ThreadSignal keyboard_input_signal;

ThreadSignal thread2_signal;

void thread1(void*) {
    while (true) {
        kthread_await(&keyboard_input_signal);

        cout << " 1";

        kthread_emit(&thread2_signal);
    }
}

void thread2(void*) {
    while (true) {
        kthread_await(&thread2_signal);
        
        cout << " 2";
    }
}

// idk what this is lmao
extern "C" void __cxa_atexit() {}

void kernel_init(void*) {
    /*
#ifdef UNIT_TESTING_BEFORE_STARTUP
    init_all_unit_tests();

    bool success = UnitTest::run_all_tests();

    if (!success) {
        Log::WARN() << "Not all tests ran were successfull!\n";
    }
#endif
    */

    PCIManager::init();

    FileSystemManager::init();

    SyscallError err;

    err = FileSystemManager::get()->mkdir("/dev", S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH, nullptr);
    if (err != ENOERR)
        Log::INFO() << "test: " << get_error_message(err) << '\n';

    err = FileSystemManager::get()->mkdir("/mnt", S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH, nullptr);
    if (err != ENOERR)
        Log::INFO() << "test: " << get_error_message(err) << '\n';

    Log::INFO() << '\n';
    FileSystemManager::get()->dbg_ls("/");
    Log::INFO() << '\n';
    FileSystemManager::get()->dbg_ls("/dev/");
    Log::INFO() << '\n';

    err = FileSystemManager::get()->mount("/dev", DevFileSystem::get());
    if (err != ENOERR)
        Log::INFO() << "test: " << get_error_message(err) << '\n';
    
    FileSystemManager::get()->dbg_ls("/dev/");
    Log::INFO() << '\n';

    err = FileSystemManager::get()->mount("/mnt", "/dev/sdb");
    if (err != ENOERR) {
        Log::INFO() << "test: " << get_error_message(err) << '\n';
        return;
    }
    
    FileSystemManager::get()->dbg_ls("/mnt/bin/");
    Log::INFO() << '\n';

    FileDescription* fd;

    err = FileSystemManager::get()->open(&fd, "/mnt/bin/test", 0, nullptr);
    if (err != ENOERR) {
        Log::INFO() << "test: " << get_error_message(err) << '\n';
        return;
    }

    Process* process = ProcessManager::get()->create();

    err = process->exec(fd);
    if (err != ENOERR) {
        Log::INFO() << "test: " << get_error_message(err) << '\n';
        return;
    }
    
    FileSystemManager::get()->close(fd);

    /*
    FileSystemManager::get()->dbg_ls("/mnt/");
    Log::INFO() << '\n';
    */

    /*
    FileSystemManager::get()->dbg_ls("/mnt/kernel/");
    Log::INFO() << '\n';
    
    FileSystemManager::get()->dbg_ls("/mnt/kernel/src/");
    Log::INFO() << '\n';

    FileDescription* fd;

    err = FileSystemManager::get()->open(&fd, "/mnt/kernel/src/main.cpp", OPENF_READ | OPENF_WRITE, nullptr);
    if (err != ENOERR) {
        Log::INFO() << "test: " << get_error_message(err) << '\n';
        return;
    }

    fd->write((void*)"This text was written from the Ampersand kernel!", 0, 48);

    char* buffer = new char[fd->get_size() + 1];
    fd->read(buffer, 0, fd->get_size());

    Log::INFO() << buffer << '\n';

    FileSystemManager::get()->close(fd);
    */

    FileSystemManager::get()->unmount("/mnt");

    for (;;);
}

static DebugTerminalOutput dbg;

#include <arch/thread.hpp>

extern "C" void kernel_main() {
    BootLoader::init();
    VirtualMemoryManager::init();
    MemoryManager::init();

    VGATextDriver::init();

    cout.add_terminal(&dbg);
    cout.add_terminal(VGATextDriver::get());
    cout.set_cursor_visible(true);
    cout.clear(TERM_BLACK);

    cout.write("Terminal output loaded\n");
    Log::INFO() << "Initializing kernel...\n";
    Log::INFO() << "Kernel: Ampersand & v" << AMPERSAND_VERSION << '\n';
    Log::INFO() << "Bootloader: " << BootLoader::get_name() << '\n';
    Log::INFO() << "Target Arch: " << ARCH_NAME << " (" << ARCH_BITS << "-bit)\n";
    
    PhysicalMemoryMap::log_memory_map();

    auto o = Log::INFO();
    o << "Available memory: ";
    o.write_size(MemoryManager::get()->total_available_memory());
    o << '\n';

    o = Log::INFO();
    auto range = PhysicalMemoryMap::get_kernel_range();
    o << Out::phex(16) << "Kernel range: " << range.base << " - " << range.limit << " (";
    o.write_size(PhysicalMemoryMap::get_kernel_range().size());
    o << ")\n";

    arch_init();

    arch_thread_set_syscall_handler([]() {
        Log::INFO() << "Syscall!!\n";
    });

    ThreadScheduler::init();

    kthread_create(thread1, nullptr);
    kthread_create(thread2, nullptr);
    kthread_create(kernel_init, nullptr);

    ThreadScheduler::get()->run();

    arch_lock_cpu();
}