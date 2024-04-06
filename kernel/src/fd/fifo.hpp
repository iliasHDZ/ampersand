#pragma once

#include "fd.hpp"

#include <common/circular.hpp>

#define FIFO_BUFFER_SIZE 8 * KiB

class FIFO : public FileDescription {
public:
    FIFO();

    u64 read(void* out, u64 offset, u64 size) override;

    u64 write(void* in, u64 offset, u64 size) override;

    bool can_read() override;

    bool can_write() override;

    bool should_block() override;

private:
    CircularBuffer cbuffer;
};