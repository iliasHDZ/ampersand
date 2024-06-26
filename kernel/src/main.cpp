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
#include "drv/vga_text.hpp"

#include "drv/debugout.hpp"

#include <arch/arch.hpp>
#include <arch/keyboard.hpp>
#include <logger.hpp>
#include "fd/manager.hpp"

// idk what this is lmao
extern "C" void __cxa_atexit() {}

CharacterDevice* keyboard = nullptr;

void keyboard_transmit_callback(u8 sc) {
    if (keyboard != nullptr)
        keyboard->device_transmit(&sc, 1);
}

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

    VGABlockDevice::init();

    keyboard = new CharacterDevice(true, false, 1 * KiB);

    DevFileSystem::get()->add_device(keyboard, DevInode::CHARACTER_DEVICE, "kbd");

    arch_set_keyboard_callback(keyboard_transmit_callback);

    SyscallError err;

    err = FileSystemManager::get()->mkdir("/dev", S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH, nullptr);
    if (err != ENOERR) {
        Log::ERR() << "Could not mkdir /dev: " << get_error_message(err) << '\n';
        panic("Error while mounting root");
    }

    err = FileSystemManager::get()->mount("/dev", DevFileSystem::get());
    if (err != ENOERR) {
        Log::ERR() << "Could not mount /dev: " << get_error_message(err) << '\n';
        panic("Error while mounting root");
    }

    FileDescription* fd;
    err = FileSystemManager::get()->open(&fd, "/dev/sdb", O_RDWR, nullptr);
    if (err != ENOERR) {
        Log::ERR() << "Could not open /dev/sdb: " << get_error_message(err) << '\n';
        panic("Error while mounting root");
    }

    err = FileSystemManager::get()->unmount("/dev", true);
    if (err != ENOERR) {
        Log::ERR() << "Could not unmount /dev: " << get_error_message(err) << '\n';
        panic("Error while mounting root");
    }

    err = FileSystemManager::get()->unmount("/", true);
    if (err != ENOERR) {
        Log::ERR() << "Could not unmount /: " << get_error_message(err) << '\n';
        panic("Error while mounting root");
    }

    err = FileSystemManager::get()->mount("/", fd, "/dev/sdb");
    if (err != ENOERR) {
        Log::ERR() << "Could not mount / from /dev/sdb: " << get_error_message(err) << '\n';
        panic("Error while mounting root");
    }

    err = FileSystemManager::get()->mkdir("/dev", S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH, nullptr);
    if (err != ENOERR && err != EEXIST) {
        Log::ERR() << "Could not mkdir /dev: " << get_error_message(err) << '\n';
        panic("Error while mounting root");
    }

    err = FileSystemManager::get()->mount("/dev", DevFileSystem::get());
    if (err != ENOERR) {
        Log::ERR() << "Could not remount /dev: " << get_error_message(err) << '\n';
        panic("Error while mounting root");
    }

    err = FileSystemManager::get()->open(&fd, "/bin/terminal", 0, nullptr);
    if (err != ENOERR) {
        Log::INFO() << "terminal: " << get_error_message(err) << '\n';
        return;
    }

    ProcessManager::init();

    Process* process = ProcessManager::get()->create();
    
    cout.disable_terminal(VGATextDriver::get());

    err = process->exec(fd);
    if (err != ENOERR) {
        Log::INFO() << "test: " << get_error_message(err) << '\n';
        return;
    }

    FileDescriptionManager::get()->close(fd);
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

    ThreadScheduler::init();

    kthread_create(kernel_init, nullptr);

    ThreadScheduler::get()->run();

    arch_lock_cpu();
}