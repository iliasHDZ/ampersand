#include "blkdev.hpp"

bool BlockDevice::has_size() {
    return true;
}

u64 MemoryBlockDevice::read(void* out, u64 offset, u64 size) {
    if (offset >= get_size())
        return 0;

    size = min(get_size() - offset, size);

    memcpy(out, (u8*)get_address() + offset, size);
    return size;
}

u64 MemoryBlockDevice::write(void* in, u64 offset, u64 size) {
    if (offset >= get_size())
        return 0;

    size = min(get_size() - offset, size);

    memcpy((u8*)get_address() + offset, in, size);
    return size;
}