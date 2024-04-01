#pragma once

#include <common.h>

class CircularBuffer {
public:
    CircularBuffer(usize size);

    ~CircularBuffer();

    CircularBuffer(const CircularBuffer&) = delete;
    CircularBuffer& operator=(const CircularBuffer&) = delete;

    u64 read(void* out, u64 size);

    u64 write(void* in, u64 size);

    bool can_read() const;

    bool can_write() const;

    usize readable_size() const;

    usize writable_size() const;

private:
    usize rd_ptr;
    usize wr_ptr;

    u8* buffer;
    usize buffer_size;
};