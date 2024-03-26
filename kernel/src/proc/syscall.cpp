#include "process.hpp"
#include <fd/manager.hpp>
#include <fs/fs_manager.hpp>
#include <fcntl.h>

isize Process::sys_read(i32 fd, void* buf, usize size) {
    if (!is_handle_open(fd))
        return -EBADF;

    // TODO: Check if buf is a valid user writable buffer

    FileDescriptionHandle* fdh = &fd_handles[fd];

    if (!(fdh->perms | OPENF_READ))
        return -EBADF;
    
    return fdh->read(buf, size);
}

isize Process::sys_write(i32 fd, void* buf, usize size) {
    if (!is_handle_open(fd))
        return -EBADF;

    // TODO: Check if buf is a valid user readable buffer

    FileDescriptionHandle* fdh = &fd_handles[fd];

    if (!(fdh->perms | OPENF_WRITE))
        return -EBADF;
    
    return fdh->write(buf, size);
}

i32 Process::sys_open(const char* path, isize oflags) {
    // TODO: Check if path is a valid user readable buffer

    u32 flags = 0;
    usize flc = 0;

    if (oflags & O_RDONLY) {
        flags |= OPENF_READ;
        flc++;
    }

    if (oflags & O_WRONLY) {
        flags |= OPENF_READ;
        flc++;
    }

    if (oflags & O_RDWR) {
        flags |= OPENF_READ | OPENF_WRITE;
        flc++;
    }

    if (flc != 1)
        return -EINVAL;
    
    if (oflags & O_CREAT)
        flags |= OPENF_CREAT;

    FileDescription* fd;
    SyscallError err = FileSystemManager::get()->open(&fd, path, flags, &creds);
    if (err != ENOERR)
        return -err;
    
    i32 fd_num = open_handle(fd, flags);
    if (fd_num < 0) {
        FileDescriptionManager::get()->close(fd);
        return fd_num;
    }
    
    return fd_num;
}

i32 Process::sys_close(i32 fd) {
    if (!is_handle_open(fd))
        return EBADF;
    
    return close_handle(fd);
}