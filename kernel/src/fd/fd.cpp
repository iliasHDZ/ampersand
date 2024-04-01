#include "fd.hpp"
#include <errno.h>

static ThreadSignal fd_event;

bool FileDescription::has_size() {
    return false;
}

u64 FileDescription::get_size() {
    return 0;
}

bool FileDescription::can_read() {
    return true;
}

bool FileDescription::can_write() {
    return true;
}

bool FileDescription::should_block() {
    return false;
}

usize FileDescription::ioctl_count(isize request) {
    return 0;
}

isize FileDescription::ioctl(isize request, usize* args) {
    return -EBADF;
}

void FileDescription::emit_event() {
    kthread_emit(&fd_event);
}

void FileDescription::await_event() {
    kthread_await(&fd_event);
}