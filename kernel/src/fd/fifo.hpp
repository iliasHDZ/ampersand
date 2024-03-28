#pragma once

#include "fd.hpp"

#define FIFO_QUEUE_SIZE 8 * KiB

class FIFO : public FileDescription {
public:
    FIFO();

    u64 read(void* out, u64 offset, u64 size) override;

    u64 write(void* in, u64 offset, u64 size) override;

    bool should_block() override;

private:
    usize readable_size() const;

    usize writable_size() const;

private:
    usize rd_ptr;
    usize wr_ptr;

    u8 buffer[FIFO_QUEUE_SIZE] = { 0 };
};