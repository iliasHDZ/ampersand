#pragma once

#include "fs.hpp"
#include <common/vec.hpp>

// This does NOT work well and should be completely rewritten!
// VirtualFs should only be used to temporarily mount as root
// VirtualFs cannot be correctly deconstructed

struct VirtualDirEntry {
    u32 inode_id;
    char name[128];
};

struct VirtualFsInode {
    Inode inode;
    void* data;
    Vec<VirtualDirEntry> entries = { 0 };

    ~VirtualFsInode();

    void retain();

    void release();

    void resize(usize size);
};

class VirtualFs : public FileSystem {
public:
    VirtualFs();

    InodeFile* create_fd(Inode* file) override;

    void free_fd(InodeFile* fd) override;

    FSFileOpenMethod get_open_method(Inode* file) override;

    FSStatus get_root(Inode* root_out) override;

    FSStatus fetch(Inode* directory, Inode* child_out, const char* name) override;

    FSStatus create(Inode* inode_out, u32 mode, UserID uid, GroupID gid) override;

    FSStatus link(Inode* directory, Inode* inode, const char* name) override;

    FSStatus unlink(Inode* directory, const char* name) override;

    FSStatus mkdir(Inode* parent, Inode* dir_out, u32 mode, UserID uid, GroupID gid, const char* name) override;

    FSStatus rmdir(Inode* parent, const char* name) override;

    FSStatus getdirentcount(Inode* directory, u32* count_out) override;

    FSStatus getdirents(Inode* directory, DirEntry* dirents_out) override;

    void unmount() override;

    const char* source_path() override;

private:
    u32 alloc_inode();

private:
    Vec<VirtualFsInode> inodes;
};