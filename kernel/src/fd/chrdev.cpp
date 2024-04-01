#include "chrdev.hpp"

CharacterDevice::CharacterDevice(bool read, bool write, usize buffer_size) {
    if (read)
        read_buffer = new CircularBuffer(buffer_size);
    if (write)
        write_buffer = new CircularBuffer(buffer_size);
}

CharacterDevice::~CharacterDevice() {
    if (read_buffer)
        delete read_buffer;
    if (write_buffer)
        delete write_buffer;
}

u64 CharacterDevice::read(void* out, u64 offset, u64 size) {
    if (read_buffer == nullptr)
        return 0;

    return read_buffer->read(out, size);
}

u64 CharacterDevice::write(void* in, u64 offset, u64 size) {
    if (write_buffer == nullptr)
        return 0;

    return write_buffer->write(in, size);
}

u64 CharacterDevice::device_transmit(void* out, u64 size) {
    if (read_buffer == nullptr)
        return 0;

    u64 ret = read_buffer->write(out, size);
    emit_event();
    return ret;
}

u64 CharacterDevice::device_receive(void* in, u64 size) {
    if (write_buffer == nullptr)
        return 0;

    u64 ret = write_buffer->read(in, size);
    emit_event();
    return ret;
}

bool CharacterDevice::can_read() {
    return read_buffer != nullptr && read_buffer->can_read();
}

bool CharacterDevice::can_write() {
    return write_buffer != nullptr && write_buffer->can_read();
}

bool CharacterDevice::should_block() {
    return true;
}