#include "fifo.hpp"

FIFO::FIFO() {}

u64 FIFO::read(void* out, u64 offset, u64 size) {
    if (rd_ptr == wr_ptr)
        return 0;

    size = min(size, (u64)readable_size());

    if (wr_ptr > rd_ptr) {
        memcpy(out, &buffer[rd_ptr], size);
        rd_ptr += size;
        return size;
    }

    usize csize = min<usize>(FIFO_QUEUE_SIZE - rd_ptr, size);
    u8* buf = (u8*)out;

    memcpy(buf, &buffer[rd_ptr], csize);
    rd_ptr = (rd_ptr + csize) % FIFO_QUEUE_SIZE;
    buf += csize;

    if (size <= FIFO_QUEUE_SIZE - rd_ptr)
        return size;

    memcpy(buf, &buffer[rd_ptr], size - csize);
    rd_ptr += size - csize;
    return size;
}

u64 FIFO::write(void* in, u64 offset, u64 size) {
    if ((wr_ptr + 1) % FIFO_QUEUE_SIZE == rd_ptr)
        return 0;

    size = min(size, (u64)writable_size());

    if (rd_ptr > wr_ptr) {
        memcpy(&buffer[wr_ptr], in, size);
        wr_ptr += size;
        return size;
    }

    usize csize = min<usize>(FIFO_QUEUE_SIZE - wr_ptr, size);
    u8* buf = (u8*)in;

    memcpy(&buffer[wr_ptr], buf, csize);
    wr_ptr = (wr_ptr + csize) % FIFO_QUEUE_SIZE;
    buf += csize;

    if (size <= FIFO_QUEUE_SIZE - wr_ptr)
        return size;

    memcpy(buf, &buffer[wr_ptr], size - csize);
    wr_ptr += size - csize;
    return size;
}

bool FIFO::should_block() {
    return true;
}

usize FIFO::readable_size() const {
    isize size = wr_ptr - rd_ptr;
    if (size >= 0)
        return size;
    
    return (FIFO_QUEUE_SIZE - rd_ptr) + wr_ptr;
}

usize FIFO::writable_size() const {
    isize size = rd_ptr - wr_ptr;
    if (size >= 0)
        return size - 1;
    
    return (FIFO_QUEUE_SIZE - wr_ptr) + rd_ptr - 1;
}