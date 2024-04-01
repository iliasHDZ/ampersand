#include "process.hpp"
#include <fd/manager.hpp>
#include <fs/fs_manager.hpp>
#include <fcntl.h>
#include <logger.hpp>
#include <data/thread.hpp>

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
        return -EBADF;
    
    return close_handle(fd);
}

isize Process::sys_lseek(i32 fd, isize offset, u32 whence) {
    if (!is_handle_open(fd))
        return -EBADF;

    FileDescriptionHandle* fdh = &fd_handles[fd];
    if (!fdh->fd->has_size())
        return -ESPIPE;
    
    switch (whence) {
    case SEEK_SET:
        fdh->access_ptr = offset;
        break;
    case SEEK_CUR:
        fdh->access_ptr += offset;
        break;
    case SEEK_END:
        fdh->access_ptr = fdh->fd->get_size() + offset;
        break;
    default:
        return -EINVAL;
    }

    return fdh->access_ptr;
}

i32 Process::sys_pipe(i32* out) {
    FileDescription* pipe = FileDescriptionManager::get()->create_pipe();

    i32 rd_end = open_handle(pipe, OPENF_READ);
    if (rd_end < 0) {
        FileDescriptionManager::get()->close(pipe);
        return rd_end;
    }

    i32 wr_end = open_handle(pipe, OPENF_WRITE);
    if (wr_end < 0) {
        FileDescriptionManager::get()->close(pipe);
        return wr_end;
    }

    out[0] = rd_end;
    out[1] = wr_end;
    return 0;
}

i32 Process::sys_dup(i32 src) {
    return duplicate_handle(src);
}

i32 Process::sys_dup2(i32 src, i32 dst) {
    return duplicate_handle(src, dst);
}

i32 Process::sys_ioctl(i32 fdn, i32 request, usize* args) {
    if (!is_handle_open(fdn))
        return -EBADF;

    FileDescription* fd = fd_handles[fdn].fd;

    if (args == nullptr)
        return fd->ioctl_count(request);

    return fd->ioctl(request, args);
}