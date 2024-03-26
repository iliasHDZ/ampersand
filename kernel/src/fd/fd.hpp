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

class InodeFile : public FileDescription {
public:
    inline InodeFile() {};

    inline void set_inode_iden(usize inode_num, FileSystem* fs) {
        this->inode_num = inode_num;
        this->fs = fs;
    }

    inline usize get_inode_num() const { return inode_num; };

    inline FileSystem* get_fs() const { return fs; };

private:
    usize inode_num = 0;
    FileSystem* fs = nullptr;
};