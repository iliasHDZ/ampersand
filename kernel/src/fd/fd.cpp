#include "fd.hpp"

bool FileDescription::has_size() {
    return false;
}

u64 FileDescription::get_size() {
    return 0;
}