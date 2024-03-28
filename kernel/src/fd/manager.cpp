#include "manager.hpp"

#include <fs/fs_manager.hpp>

#include "fifo.hpp"

FileDescriptionManager fdm_instance;

FileDescription* FileDescriptionManager::fetch_inode_fd(usize inode_num, FileSystem* fs) {
    for (auto& ofd : fds) {
        if (ofd.src != FileDescriptionSource::FILESYSTEM)
            continue;

        InodeFile* ifd = (InodeFile*)(ofd.fd);

        if (ifd->get_inode_num() == inode_num && ifd->get_fs() == fs) {
            ofd.refcount++;
            return ifd;
        }
    }

    return nullptr;
}

FileDescription* FileDescriptionManager::create_pipe() {
    FIFO* pipe = new FIFO();
    save_fd(pipe, FileDescriptionSource::PIPE);

    return pipe;
}

bool FileDescriptionManager::is_filesystem_busy(FileSystem* fs) {
    for (auto& ofd : fds) {
        if (ofd.src != FileDescriptionSource::FILESYSTEM)
            continue;

        if (((InodeFile*)ofd.fd)->get_fs() == fs)
            return true;
    }
}

void FileDescriptionManager::save_fd(FileDescription* fd, FileDescriptionSource src) {
    fds.append(OpenFileDescription {src, 0, fd});
}

void FileDescriptionManager::open(FileDescription* fd) {
    for (usize i; i < fds.size(); i++) {
        if (fds[i].fd == fd) {
            fds[i].refcount++;
            return;
        }
    }
}

SyscallError FileDescriptionManager::close(FileDescription* fd) {
    OpenFileDescription* ofd = nullptr;
    usize idx = 0;

    for (; idx < fds.size(); idx++) {
        if (fds[idx].fd == fd) {
            ofd = &fds[idx];
            break;
        }
    }

    if (ofd == nullptr)
        return EBADF;
    
    if (ofd->refcount > 0)
        ofd->refcount--;
    
    if (ofd->refcount > 0)
        return ENOERR;

    SyscallError err = ENOERR;

    switch (ofd->src) {
    case FileDescriptionSource::FILESYSTEM:
        err = FileSystemManager::get()->close((InodeFile*)fd);
        break;
    case FileDescriptionSource::PIPE:
        delete fd;
        break;
    default: break;
    }

    fds.remove(idx);
    return err;
}

FileDescriptionManager* FileDescriptionManager::get() {
    return &fdm_instance;
}

void FileDescriptionManager::init() {
    // Unused lmao
}