#include "fd.hpp"
#include <errno.h>

bool FileDescription::has_size() {
    return false;
}

u64 FileDescription::get_size() {
    return 0;
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