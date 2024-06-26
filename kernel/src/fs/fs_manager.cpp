#include "fs_manager.hpp"
#include <fd/manager.hpp>
#include "virtualfs.hpp"
#include <logger.hpp>
#include <fcntl.h>

#include "ext2.hpp"

void DirEntry::set_name(const char* name, isize len) {
    len = len == -1 ? strlen(name) : len;
    if (len > 256) {
        panic("DirEntry: Directory entry name too long");
        return;
    }

    memcpy(this->name, name, len);
    this->name[len] = 0;
}

static const char* error_messages[] = {
    "No error",
    "Argument list too long",
    "Permission denied",
    "Address in use",
    "Address not available",
    "Address family not supported",
    "Resource unavailable, try again",
    "Connection already in progress",
    "Bad file descriptor",
    "Bad message",
    "Device or resource busy",
    "Operation canceled",
    "No child processes",
    "Connection aborted",
    "Connection refused",
    "Connection reset",
    "Resource deadlock would occur",
    "Destination address required",
    "Mathematics argument out of domain of function",
    "Reserved",
    "File exists",
    "Bad address",
    "File too large",
    "Host is unreachable",
    "Identifier removed",
    "Illegal byte sequence",
    "Operation in progress",
    "Interrupted function",
    "Invalid argument",
    "I/O error",
    "Socket is connected",
    "Is a directory",
    "Too many levels of symbolic links",
    "File descriptor value too large",
    "Too many links",
    "Message too large",
    "Reserved",
    "Filename too long",
    "Network is down",
    "Connection aborted by network",
    "Network unreachable",
    "Too many files open in system",
    "No buffer space available",
    "No message is available on the STREAM head read queue",
    "No such device",
    "No such file or directory",
    "Executable file format error",
    "No locks available",
    "Reserved",
    "Not enough space",
    "No message of the desired type",
    "Protocol not available",
    "No space left on device",
    "No STREAM resources",
    "Not a STREAM",
    "Functionality not supported",
    "The socket is not connected",
    "Not a directory",
    "Directory not empty",
    "State not recoverable",
    "Not a socket",
    "Not supported",
    "Inappropriate I/O control operation",
    "No such device or address",
    "Operation not supported on socket",
    "Value too large to be stored in data type",
    "Previous owner died",
    "Operation not permitted",
    "Broken pipe",
    "Protocol error",
    "Protocol not supported",
    "Protocol wrong type for socket",
    "Result too large",
    "Read-only file system",
    "Invalid seek",
    "No such process",
    "Reserved",
    "Stream ioctl() timeout",
    "Connection timed out",
    "Text file busy",
    "Operation would block",
    "Cross-device link"
};

const char* get_error_message(usize err) {
    if (err == 0x80)
        return "Invalid file system type";

    if (err == 0xff)
        return "General error";

    return error_messages[err];
}

FileSystemManager fsman_instance;

Mount::Mount(Inode mountpoint, FileSystem* fs, bool should_delete_fs)
    : mountpoint(mountpoint), fs(fs), should_delete_fs(should_delete_fs) {}

Inode& Mount::get_mountpoint() {
    return mountpoint;
}

FileSystem* Mount::get_fs() {
    return fs;
}

FileSystemManager::FileSystemManager() {}

SyscallError FileSystemManager::mount(const Path& mntpath, const Path& blkpath) {
    FileDescription* fd;

    SyscallError err = open(&fd, blkpath, O_RDWR, nullptr);
    if (err != ENOERR)
        return err;

    return mount(mntpath, fd, blkpath);
}

SyscallError FileSystemManager::mount(const Path& mntpath, FileDescription* fd, const Path& srcpath) {
    FileSystem* fs = create_fs_from_fd(fd, srcpath);
    if (fs == nullptr) {
        return EINVFS;
    }

    SyscallError err = mount(mntpath, fs, true);
    if (err != ENOERR) {
        fs->unmount();
        delete fs;
        return err;
    }

    return ENOERR;
}

SyscallError FileSystemManager::mount(const Path& path, FileSystem* fs, bool should_delete_fs) {
    Inode dir;

    if (path.is_root())
        dir = { .inode_id = 0, .filesystem = nullptr };
    else {
        SyscallError err = get_inode(&dir, path, nullptr);
        if (err != ENOERR)
            return err;

        if (!S_ISDIR(dir.mode))
            return ENOTDIR;
    }
    
    Mount* mount = get_mount_at_inode(&dir);

    if (mount)
        return EEXIST;

    const char* src = fs->source_path();
    src = src ? src : "none";

    Log::INFO("FileSystemManager") << "Mounted " << src << " on " << path << " type " << fs->get_name() << '\n';

    fs->set_dev_no(dev_counter++);
    
    Mount* mnt = new Mount { dir, fs, should_delete_fs };
    mounts.append(mnt);

    if (path.is_root())
        rootmount = mnt;

    return ENOERR;
}

SyscallError FileSystemManager::unmount(const Path& path, bool force) {
    Inode dir;

    SyscallError err = get_inode(&dir, path, nullptr);
    if (err != ENOERR)
        return err;

    Mount* mount = get_mount_with_root(&dir);

    if (mount == nullptr)
        return ENOENT;

    if (!force) {
        for (auto& mnt : mounts) {
            if (mnt->get_mountpoint().filesystem == mount->get_fs())
                return EBUSY;
        }

        if (FileDescriptionManager::get()->is_filesystem_busy(mount->get_fs()))
            return EBUSY;
    }

    mount->get_fs()->unmount();
    if (mount->should_delete_fs)
        delete mount->get_fs();

    for (usize i = 0; i < mounts.size(); i++) {
        if (mounts[i] == mount) {
            delete mounts[i];
            mounts.remove(i);
            break;
        }
    }

    auto o = Log::INFO("FileSystemManager");
    if (force)
        o << "Forcibly unmounted ";
    else
        o << "Unmounted ";
    o << path << '\n';

    return ENOERR;
}

SyscallError FileSystemManager::mkdir(const Path& path, usize mode, Credentials* creds) {
    SyscallError err;

    mode = (mode & ~S_IFMT) | S_IFDIR;

    Inode dummy;
    return create_inode(&dummy, path, mode, creds);
}

SyscallError FileSystemManager::rmdir(const Path& path, Credentials* creds) {
    SyscallError err;

    Inode parent;
    err = get_inode(&parent, path.parent(), creds);
    if (err != ENOERR)
        return err;
    
    if (!S_ISDIR(parent.mode))
        return ENOTDIR;

    if (!creds->may_write(&parent))
        return EACCES;
    
    return status_to_syscallerror(parent.filesystem->rmdir(&parent, path.filename()));
}

SyscallError FileSystemManager::open(FileDescription** fdout, const Path& path, u32 flags, Credentials* creds) {
    Inode inode;

    SyscallError err = get_inode(&inode, path, creds);
    if (err != ENOERR) {
        if (!(flags & O_CREAT))
            return err;
        
        err = create_inode(&inode, path, S_IFREG | S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH, creds);
        if (err != ENOERR)
            return err;
    }

    if (creds) {
        if (((flags & O_RDONLY) || (flags & O_RDWR)) && !creds->may_read(&inode))
            return EACCES;
        
        if (((flags & O_WRONLY) || (flags & O_RDWR)) && !creds->may_write(&inode))
            return EACCES;
    }

    FileDescription* fd = FileDescriptionManager::get()->fetch_inode_fd(inode.inode_id, inode.filesystem);

    if (fd) {
        *fdout = fd;
        return ENOERR;
    }

    FileSystem* fs = inode.filesystem;

    FSFileOpenMethod method = fs->get_open_method(&inode);

    switch (method) {
    case FSFileOpenMethod::NAMED_PIPE:
        Log::ERR("FileSystemManager") << "Cannot open named pipes: not supported";
        return 0xff;
    case FSFileOpenMethod::ERR_EXIST:
        return EEXIST;
    case FSFileOpenMethod::ERR_GENERAL:
        return 0xff;
    case FSFileOpenMethod::ERR_ACCESS:
        return EACCES;
    }

    InodeFile* inof = fs->create_fd(&inode);
    inof->set_inode_iden(inode.inode_id, fs);
    
    FileDescriptionManager::get()->save_fd(inof, FileDescriptionSource::FILESYSTEM);

    if (creds)
        inof->may_exec = creds->may_exec(&inode);
    else
        inof->may_exec = true;
    
    *fdout = inof;
    return ENOERR;
}

SyscallError FileSystemManager::close(InodeFile* fd) {
    fd->get_fs()->free_fd(fd);
    return ENOERR;
}

SyscallError FileSystemManager::link(const Path& path, const Path& newpath, Credentials* creds) {
    SyscallError err;

    Inode inode;
    Inode dir;

    err = get_inode(&inode, path, creds);
    if (err != ENOERR)
        return err;
    
    err = get_inode(&dir, newpath, creds);
    if (err != ENOENT)
        return (err == ENOERR) ? EEXIST : err;
    
    err = get_inode(&dir, newpath.parent(), creds);
    if (err != ENOERR)
        return err;

    if (!S_ISDIR(dir.mode))
        return ENOTDIR;

    if (!creds->may_write(&dir))
        return EACCES;

    return status_to_syscallerror(dir.filesystem->link(&dir, &inode, newpath.filename()));
}

SyscallError FileSystemManager::unlink(const Path& path, Credentials* creds) {
    SyscallError err;
    Inode inode;

    err = get_inode(&inode, path, creds);
    if (err != ENOERR)
        return err;

    err = get_inode(&inode, path.parent(), creds);
    if (err != ENOERR)
        return err;

    if (!creds->may_write(&inode))
        return EACCES;

    return status_to_syscallerror(inode.filesystem->unlink(&inode, path.filename()));
}

SyscallError FileSystemManager::getdirentcount(const Path& path, u32* count_out, Credentials* creds) {
    Inode dir;

    SyscallError err = get_inode(&dir, path, creds);
    if (err != ENOERR)
        return err;

    if (!S_ISDIR(dir.mode))
        return ENOTDIR;

    if (creds && !creds->may_read(&dir))
        return EACCES;
    
    FSStatus status = dir.filesystem->getdirentcount(&dir, count_out);
    return status_to_syscallerror(status);
}

SyscallError FileSystemManager::getdirents(const Path& path, DirEntry* dirents_out, Credentials* creds) {
    Inode dir;

    SyscallError err = get_inode(&dir, path, creds);
    if (err != ENOERR)
        return err;

    if (!S_ISDIR(dir.mode))
        return ENOTDIR;

    if (creds && !creds->may_read(&dir))
        return EACCES;
    
    FSStatus status = dir.filesystem->getdirents(&dir, dirents_out);
    return status_to_syscallerror(status);
}

SyscallError FileSystemManager::stat(const Path& path, FSStat* stat_out, Credentials* creds) {
    Inode inode;

    SyscallError err = get_inode(&inode, path, creds);
    if (err != ENOERR)
        return err;

    // TODO: Check if it requires any other permissions

    stat_out->dev     = inode.filesystem->get_dev_no();
    stat_out->ino     = inode.inode_id;
    stat_out->mode    = inode.mode;
    stat_out->nlink   = inode.links;
    stat_out->uid     = inode.uid;
    stat_out->gid     = inode.gid;
    stat_out->rdev    = 0; // TODO: Implement this
    stat_out->size    = inode.size;
    stat_out->blksize = 0; // TODO: Implement this
    stat_out->blocks  = 0; // TODO: Implement this

    return ENOERR;
}

Mount* FileSystemManager::get_mount_at_inode(Inode* inode) {
    for (auto& mount : mounts) {
        if (mount->get_mountpoint() == *inode)
            return mount;
    }

    return nullptr;
}
    
Mount* FileSystemManager::get_mount_with_root(Inode* inode) {
    for (auto& mount : mounts) {
        Inode root;
        if (mount->get_fs()->get_root(&root) != FSStatus::SUCCESS)
            continue;

        if (root == *inode)
            return mount;
    }

    return nullptr;
}

SyscallError FileSystemManager::get_inode(Inode* inode_out, const Path& path, Credentials* creds) {
    FSStatus status;
    Inode inode;

    status = rootmount->get_fs()->get_root(&inode);
    if (status != FSStatus::SUCCESS)
        return status_to_syscallerror(status);

    for (usize i = 0; i < path.segment_count(); i++) {
        const char* component = path[i];
        
        if (!S_ISDIR(inode.mode))
            return ENOTDIR;

        if (creds && !creds->may_exec(&inode))
            return EACCES;

        if (strlen(component) > 255)
            return ENAMETOOLONG;

        if (streq(component, "..")) {
            Mount* mnt = get_mount_with_root(&inode);
            if (mnt && mnt->get_mountpoint().filesystem)
                inode = mnt->get_mountpoint();
        }
        
        status = inode.fetch(&inode, component);
        if (status != FSStatus::SUCCESS)
            return status_to_syscallerror(status);

        Mount* mount = get_mount_at_inode(&inode);
        if (mount) {
            status = mount->get_fs()->get_root(&inode);
            if (status != FSStatus::SUCCESS)
                return status_to_syscallerror(status);
        }
    }

    *inode_out = inode;
    return ENOERR;
}

SyscallError FileSystemManager::create_inode(Inode* inode_out, const Path& path, u32 mode, Credentials* creds) {
    SyscallError err;

    Inode parent;
    err = get_inode(&parent, path.parent(), creds);
    if (err != ENOERR)
        return err;

    if (!S_ISDIR(parent.mode))
        return ENOTDIR;
    
    if (creds && !creds->may_write(&parent))
        return EACCES;
    
    Inode inode;
    err = get_inode(&inode, path, creds);
    if (err != ENOENT)
        return (err == ENOERR) ? EEXIST : err;

    UserID  uid = 0;
    GroupID gid = 0;

    if (creds) {
        uid = creds->get_uid();
        gid = creds->get_pgid();
    }
    
    FSStatus status;

    if ((mode & S_IFMT) == S_IFDIR) {
        status = parent.filesystem->mkdir(&parent, &inode, mode, uid, gid, path.filename());
        if (status != FSStatus::SUCCESS)
            return status_to_syscallerror(status);
        
        *inode_out = inode;
        return ENOERR;
    }
    
    status = parent.filesystem->create(&inode, mode, uid, gid);
    if (status != FSStatus::SUCCESS)
        return status_to_syscallerror(status);
    
    status = parent.filesystem->link(&parent, &inode, path.filename());
    if (status != FSStatus::SUCCESS)
        return status_to_syscallerror(status);
    
    *inode_out = inode;
    return ENOERR;
}

SyscallError FileSystemManager::status_to_syscallerror(FSStatus status) const {
    switch (status) {
    case FSStatus::SUCCESS:
        return ENOERR;
    case FSStatus::NO_ENTRY:
        return ENOENT;
    case FSStatus::NO_SPACE:
        return ENOSPC;
    case FSStatus::EXISTS:
        return EEXIST;
    case FSStatus::NOT_EMPTY:
        return ENOTEMPTY;
    case FSStatus::NOT_DIR:
        return ENOTDIR;
    default:
    case FSStatus::NOT_ALLOWED:
        return 0xff;
    };
}

FileSystem* FileSystemManager::create_fs_from_fd(FileDescription* fd, const Path& srcpath) {
    if (Ext2FileSystem::is_of_type(fd))
        return new Ext2FileSystem(fd, ""); // TODO: Fix source path!

    return nullptr;
}

void FileSystemManager::init_internal() {
    Log::INFO("FileSystemManager") << "Initializing filesystem...\n";

    VirtualFs* tempfs = new VirtualFs();

    tempfs->set_dev_no(dev_counter++);

    Mount* mount = new Mount { { .inode_id = 0, .filesystem = nullptr }, tempfs };

    fsman_instance.mounts.append(mount);
    fsman_instance.rootmount = mount;
}

void FileSystemManager::init() {
    fsman_instance.init_internal();
}

FileSystemManager* FileSystemManager::get() {
    return &fsman_instance;
}
