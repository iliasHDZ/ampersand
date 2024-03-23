#pragma once

#include <common.h>

class FileSystem;

class FileDescription {
public:
    virtual bool has_size();

    virtual u64 get_size();

    virtual u64 read(void* out, u64 offset, u64 size) = 0;

    virtual u64 write(void* in, u64 offset, u64 size) = 0;

    bool may_exec = false;
};