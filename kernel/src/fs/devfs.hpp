#pragma once

#include "fs.hpp"
#include <fd/blkdev.hpp>
#include <fd/chrdev.hpp>
#include <common/vec.hpp>
#include <proc/credentials.hpp>

#define DEVFS_INODE_MAX_NAME_LEN 32

struct DevInode {
    enum DevInodeType {
        BLOCK_DEVICE,
        CHARACTER_DEVICE
    };

    u32 inode_id;
    DevInodeType type;
    InodeFile* device;
    char name[DEVFS_INODE_MAX_NAME_LEN + 1];

    void to_inode(FileSystem* fs, Inode* inode_out) const;
};

class DevFileSystem : public FileSystem {
public:
    DevFileSystem();
    
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

    void add_device(InodeFile* device, DevInode::DevInodeType type, const char* name);

public:
    static DevFileSystem* get();

private:
    DevInode* get_device_with_id(u32 id);
    
    DevInode* get_device_with_name(const char* name);

private:
    Vec<DevInode> devices;

    u32 inode_id_counter = 2;

};