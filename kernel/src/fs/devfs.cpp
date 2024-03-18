#include "devfs.hpp"

static constexpr UserID  dev_file_uid = 0;
static constexpr GroupID dev_file_gid = 0;

static DevFileSystem devfs_instance;

void DevInode::to_inode(FileSystem* fs, Inode* out) const {
    out->inode_id   = inode_id;
    out->filesystem = fs;
    out->size = device->get_size();
    out->mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IFBLK;
    out->uid  = dev_file_uid;
    out->gid  = dev_file_gid;
}

DevFileSystem::DevFileSystem()
    : FileSystem("devfs") {}

FileDescription* DevFileSystem::create_fd(Inode* file) {
    DevInode* device = get_device_with_id(file->inode_id);

    if (device == nullptr)
        return nullptr;

    return device->device;
}

void DevFileSystem::free_fd(FileDescription* fd) {}

FSFileOpenMethod DevFileSystem::get_open_method(Inode* file) {
    if (file->inode_id == 1)
        return FSFileOpenMethod::ERR_GENERAL;

    if (get_device_with_id(file->inode_id))
        return FSFileOpenMethod::USE_FILE_DESCRIPTION;

    return FSFileOpenMethod::ERR_GENERAL;
}

FSStatus DevFileSystem::get_root(Inode* root) {
    root->inode_id   = 1;
    root->filesystem = this;

    root->size  = 0;
    root->mode  = S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH | S_IFDIR;
    root->uid   = dev_file_uid;
    root->gid   = dev_file_gid;
    root->links = 0;

    return FSStatus::SUCCESS;
}

FSStatus DevFileSystem::fetch(Inode* dir, Inode* out, const char* name) {
    if (dir->inode_id != 1)
        return FSStatus::NOT_ALLOWED;

    if (streq(name, ".") || streq(name, ".."))
        return get_root(out);

    DevInode* device = get_device_with_name(name);

    if (device == nullptr)
        return FSStatus::NO_ENTRY;

    device->to_inode(this, out);

    return FSStatus::SUCCESS;
}

FSStatus DevFileSystem::create(Inode* inode_out, u32 mode, UserID uid, GroupID gid) {
    return FSStatus::NOT_ALLOWED;
}

FSStatus DevFileSystem::link(Inode* directory, Inode* inode, const char* name) {
    return FSStatus::NOT_ALLOWED;
}

FSStatus DevFileSystem::unlink(Inode* directory, const char* name) {
    return FSStatus::NOT_ALLOWED;
}

FSStatus DevFileSystem::mkdir(Inode* parent, Inode* dir_out, u32 mode, UserID uid, GroupID gid, const char* name)  {
    return FSStatus::NOT_ALLOWED;
}

FSStatus DevFileSystem::rmdir(Inode* parent, const char* name)  {
    return FSStatus::NOT_ALLOWED;
}

FSStatus DevFileSystem::getdirentcount(Inode* dir, u32* count_out) {
    if (dir->inode_id != 1)
        return FSStatus::NO_ENTRY;

    *count_out = devices.size() + 2;
    return FSStatus::SUCCESS;
}

FSStatus DevFileSystem::getdirents(Inode* dir, DirEntry* dirents_out) {
    if (dir->inode_id != 1)
        return FSStatus::NO_ENTRY;
    
    dirents_out[0].set_name(".");
    dirents_out[1].set_name("..");

    Inode root;
    FSStatus status = get_root(&root);
    if (status != FSStatus::SUCCESS)
        return status;
    
    dirents_out[0].inode = root;
    dirents_out[1].inode = root;

    u32 i = 2;
    for (auto& device : devices) {
        auto& ent = dirents_out[i++];

        ent.set_name(device.name);
        device.to_inode(this, &ent.inode);
    }
    
    return FSStatus::SUCCESS;
}

void DevFileSystem::unmount() {}

const char* DevFileSystem::source_path() {
    return nullptr;
}

void DevFileSystem::add_block_device(BlockDevice* device, const char* name) {
    if (get_device_with_name(name) != nullptr) {
        panic("DevFileSystem: Multiple device files with the same name");
        return;
    }

    DevInode inode;

    inode.inode_id = inode_id_counter++;
    inode.device   = device;

    usize len = strlen(name);

    if (len >= DEVFS_INODE_MAX_NAME_LEN) {
        panic("DevFileSystem: Device file name is longer than DEVFS_INODE_MAX_NAME_LEN");
        return;
    }

    memcpy(inode.name, name, len);
    inode.name[len] = 0;

    devices.append(inode);
}

DevFileSystem* DevFileSystem::get() {
    return &devfs_instance;
}

DevInode* DevFileSystem::get_device_with_id(u32 id) {
    for (auto& device : devices) {
        if (device.inode_id == id)
            return &device;
    }

    return nullptr;
}

DevInode* DevFileSystem::get_device_with_name(const char* name) {
    for (auto& device : devices) {
        if (streq(device.name, name))
            return &device;
    }

    return nullptr;
}