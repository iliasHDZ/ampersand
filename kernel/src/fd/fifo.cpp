#include "fifo.hpp"

FIFO::FIFO()
    : cbuffer(FIFO_BUFFER_SIZE) {}

u64 FIFO::read(void* out, u64 offset, u64 size) {
    if (!can_read())
        return 0;

    u64 ret = cbuffer.read(out, offset, size);
    emit_event();
    return ret;
}

u64 FIFO::write(void* in, u64 offset, u64 size) {
    if (!can_write())
        return 0;

    u64 ret = cbuffer.write(in, offset, size);
    emit_event();
    return ret;
}

bool FIFO::can_read() {
    return cbuffer.can_read();
}

bool FIFO::can_write() {
    return cbuffer.can_write();
}

bool FIFO::should_block() {
    return true;
}