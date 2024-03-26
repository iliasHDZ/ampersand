#pragma once

#include "fd.hpp"

#include <data/vec.hpp>
#include <proc/error.hpp>

enum class FileDescriptionSource {
    FILESYSTEM
};

struct OpenFileDescription {
    FileDescriptionSource src;
    usize refcount;
    FileDescription* fd;
};

class FileDescriptionManager {
public:
    FileDescription* fetch_inode_fd(usize inode_num, FileSystem* fs);

    bool is_filesystem_busy(FileSystem* fs);

    void save_fd(FileDescription* fd, FileDescriptionSource src);

    SyscallError close(FileDescription* fd);

public:
    static FileDescriptionManager* get();

private:

    static void init();

private:
    Vec<OpenFileDescription> fds;

};