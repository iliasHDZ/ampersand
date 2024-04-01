#include "circular.hpp"

CircularBuffer::CircularBuffer(usize size) {
    buffer_size = size;
    buffer = new u8[size];
}

CircularBuffer::~CircularBuffer() {
    delete[] buffer;
}

u64 CircularBuffer::read(void* out, u64 size) {
    size = min(size, (u64)readable_size());

    if (wr_ptr > rd_ptr) {
        memcpy(out, &buffer[rd_ptr], size);
        rd_ptr += size;
        return size;
    }

    usize csize = min<usize>(buffer_size - rd_ptr, size);
    u8* buf = (u8*)out;

    memcpy(buf, &buffer[rd_ptr], csize);
    rd_ptr = (rd_ptr + csize) % buffer_size;
    buf += csize;

    if (size <= buffer_size - rd_ptr)
        return size;

    memcpy(buf, &buffer[rd_ptr], size - csize);
    rd_ptr += size - csize;

    return size;
}

u64 CircularBuffer::write(void* in, u64 size) {
    size = min(size, (u64)writable_size());

    if (rd_ptr > wr_ptr) {
        memcpy(&buffer[wr_ptr], in, size);
        wr_ptr += size;
        return size;
    }

    usize csize = min<usize>(buffer_size - wr_ptr, size);
    u8* buf = (u8*)in;

    memcpy(&buffer[wr_ptr], buf, csize);
    wr_ptr = (wr_ptr + csize) % buffer_size;
    buf += csize;

    if (size <= buffer_size - wr_ptr)
        return size;

    memcpy(buf, &buffer[wr_ptr], size - csize);
    wr_ptr += size - csize;

    return size;
}

bool CircularBuffer::can_read() const {
    return rd_ptr != wr_ptr;
}

bool CircularBuffer::can_write() const {
    return (wr_ptr + 1) % buffer_size != rd_ptr;
}

usize CircularBuffer::readable_size() const {
    isize size = wr_ptr - rd_ptr;
    if (size >= 0)
        return size;
    
    return (buffer_size - rd_ptr) + wr_ptr;
}

usize CircularBuffer::writable_size() const {
    isize size = rd_ptr - wr_ptr;
    if (size >= 0)
        return size - 1;
    
    return (buffer_size - wr_ptr) + rd_ptr - 1;
}