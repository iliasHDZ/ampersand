#pragma once

#include "fd.hpp"

class BlockDevice : public FileDescription {
public:
    bool has_size() override;

    virtual u64 get_size() = 0;

    virtual u64 read(void* out, u64 offset, u64 size) = 0;

    virtual u64 write(void* in, u64 offset, u64 size) = 0;

};