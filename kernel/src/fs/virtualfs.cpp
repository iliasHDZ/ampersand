#include "virtualfs.hpp"

VirtualFsInode::~VirtualFsInode() {
    if (data)
        delete data;
    data = nullptr;
}

void VirtualFsInode::retain() {
    inode.links++;
}

void VirtualFsInode::release() {
    inode.links--;
    if (inode.links == 0) {
        if (data)
            delete data;
        data = nullptr;

        inode.mode = 0;
    }
}

void VirtualFsInode::resize(usize size) {
    if (size == 0 && data)
        delete data;
    else if (!data)
        data = kmalloc(size);
    else
        data = krealloc(data, size);

    inode.size = size;
}

VirtualFs::VirtualFs()
    : FileSystem("virtualfs")
{
    inodes.append({ .inode = { .inode_id = 0, .mode = 0 }, .data = nullptr });
    
    u32 mode = S_IFDIR | S_IRWXU | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH;

    Inode root;
    create(&root, mode, 0, 0);

    link(&root, &root, ".");
    link(&root, &root, "..");
}

FileDescription* VirtualFs::create_fd(Inode* file) {
    return nullptr;
}

void VirtualFs::free_fd(FileDescription* fd) {
    
}

FSFileOpenMethod VirtualFs::get_open_method(Inode* file) {
    return FSFileOpenMethod::ERR_GENERAL;
}

FSStatus VirtualFs::get_root(Inode* root_out) {
    *root_out = inodes[1].inode;
    return FSStatus::SUCCESS;
}

FSStatus VirtualFs::fetch(Inode* dir, Inode* child_out, const char* name) {
    u32 dir_id = dir->inode_id;
    VirtualFsInode* vdir = &inodes[dir_id];

    for (auto& ent : vdir->entries) {
        if (streq(ent.name, name)) {
            *child_out = inodes[ent.inode_id].inode;
            return FSStatus::SUCCESS;
        }
    }

    return FSStatus::NO_ENTRY;
}

FSStatus VirtualFs::create(Inode* inode_out, u32 mode, UserID uid, GroupID gid) {
    u32 id = alloc_inode();

    VirtualFsInode* vinode = &inodes[id];
    Inode* inode = &(vinode->inode);

    inode->inode_id = id;
    inode->filesystem = this;
    inode->mode = mode;
    inode->uid = uid;
    inode->gid = gid;

    *inode_out = *inode;

    return FSStatus::SUCCESS;
}

FSStatus VirtualFs::link(Inode* dir, Inode* inode, const char* name) {
    VirtualFsInode* vdir = &inodes[dir->inode_id];

    usize len = strlen(name);
    if (len > 127)
        panic("VirtualFs: Directory entry name too long. Can be max 127 characters.");

    VirtualDirEntry ent;
    ent.inode_id = inode->inode_id;
    memcpy(ent.name, name, len);
    ent.name[len] = 0;

    inodes[inode->inode_id].retain();

    vdir->entries.append(ent);
    return FSStatus::SUCCESS;
}

FSStatus VirtualFs::unlink(Inode* dir, const char* name) {
    VirtualFsInode* vdir = &inodes[dir->inode_id];

    for (usize i = 0; i < vdir->entries.size(); i++) {
        auto& entry = vdir->entries[i];

        if (streq(entry.name, name)) {
            inodes[entry.inode_id].release();
            vdir->entries.remove(i);
            return FSStatus::SUCCESS;
        }
    }

    return FSStatus::NO_ENTRY;
}

FSStatus VirtualFs::mkdir(Inode* parent, Inode* dir_out, u32 mode, UserID uid, GroupID gid, const char* name) {
    Inode dir;
    FSStatus status = create(&dir, (mode & ~S_IFMT) | S_IFDIR, uid, gid);
    if (status != FSStatus::SUCCESS)
        return status;

    link(parent, &dir, name);
    *dir_out = dir;

    link(&dir, &dir, ".");
    link(&dir, parent, "..");

    return FSStatus::SUCCESS;
}

FSStatus VirtualFs::rmdir(Inode* parent, const char* name) {
    return FSStatus::NOT_ALLOWED;
}

FSStatus VirtualFs::getdirentcount(Inode* dir, u32* count_out) {
    VirtualFsInode* vdir = &inodes[dir->inode_id];

    *count_out = vdir->entries.size();
    return FSStatus::SUCCESS;
}

FSStatus VirtualFs::getdirents(Inode* dir, DirEntry* dirents_out) {
    VirtualFsInode* vdir = &inodes[dir->inode_id];

    for (usize i = 0; i < vdir->entries.size(); i++) {
        auto& entry  = vdir->entries[i];
        auto& entout = dirents_out[i];

        entout.set_name(entry.name);
        entout.inode = inodes[entry.inode_id].inode;
    }

    return FSStatus::SUCCESS;
}

void VirtualFs::unmount() {}

const char* VirtualFs::source_path() {
    return nullptr;
}

u32 VirtualFs::alloc_inode() {
    for (u32 i = 1; i < inodes.size(); i++) {
        if (inodes[i].inode.mode == 0)
            return i;
    }

    u32 ret = inodes.size();
    inodes.append({ .inode = { .mode = 0 }, .data = nullptr });

    return ret;
}