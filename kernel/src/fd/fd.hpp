#pragma once

#include <common.h>
#include <data/thread.hpp>

class FileSystem;

class FileDescription {
public:
    virtual ~FileDescription();

    virtual bool has_size();

    virtual u64 get_size();

    virtual u64 read(void* out, u64 offset, u64 size) = 0;

    virtual u64 write(void* in, u64 offset, u64 size) = 0;

    virtual bool can_read();

    virtual bool can_write();

    virtual bool should_block();

    virtual usize ioctl_count(isize request);

    virtual isize ioctl(isize request, usize* args);

    void emit_event();

public:
    static void await_event();

public:
    bool may_exec = false;

    Mutex access_mutex;
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