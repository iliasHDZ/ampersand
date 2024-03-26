#pragma once

#include "fd.hpp"

class BlockDevice : public InodeFile {
public:
    bool has_size() override;

    virtual u64 get_size() = 0;

    virtual u64 read(void* out, u64 offset, u64 size) = 0;

    virtual u64 write(void* in, u64 offset, u64 size) = 0;

};

class MemoryBlockDevice : public BlockDevice {
public:
    virtual void* get_address() = 0;

    virtual u64 get_size() = 0;

    u64 read(void* out, u64 offset, u64 size) override;

    u64 write(void* in, u64 offset, u64 size) override;
};