#pragma once

#include "fd.hpp"
#include <common/circular.hpp>

class CharacterDevice : public InodeFile {
public:
    CharacterDevice(bool read, bool write, usize buffer_size);

    virtual ~CharacterDevice();

    u64 read(void* out, u64 offset, u64 size) override;

    u64 write(void* in, u64 offset, u64 size) override;

    u64 device_transmit(void* out, u64 size);

    u64 device_receive(void* in, u64 size);

    bool can_read() override;

    bool can_write() override;

    bool should_block() override;

private:
    CircularBuffer* read_buffer  = nullptr;
    CircularBuffer* write_buffer = nullptr;

};