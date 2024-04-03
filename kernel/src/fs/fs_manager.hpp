#include <common.h>
#include <fd/fd.hpp>
#include <proc/credentials.hpp>
#include <data/path.hpp>
#include <proc/error.hpp>
#include "fs.hpp"

#define MAX_DIRENT_NAME 255

const char* get_error_message(usize err);

struct FSStat {
    usize dev;
    usize ino;
    usize mode;
    usize nlink;
    UserID uid;
    GroupID gid;
    usize rdev;
    usize size;
    usize blksize;
    usize blocks;
};

class Mount {
public:
    Mount(Inode mountpoint, FileSystem* fs, bool should_delete_fs = false);

    Inode& get_mountpoint();

    FileSystem* get_fs();

public:
    bool should_delete_fs;

private:
    Inode mountpoint;
    FileSystem* fs;
};

class FileSystemManager {
public:
    FileSystemManager();

    SyscallError mount(const Path& mntpath, const Path& blkpath);

    SyscallError mount(const Path& mntpath, FileDescription* fd, const Path& srcpath = nullptr);

    SyscallError mount(const Path& path, FileSystem* fs, bool should_delete_fs = false);

    SyscallError unmount(const Path& path, bool force = false);

    SyscallError mkdir(const Path& path, usize mode, Credentials* creds);

    // RMDIR HAS NOT BEEN TESTED!
    SyscallError rmdir(const Path& path, Credentials* creds);

    SyscallError open(FileDescription** fdout, const Path& path, u32 flags, Credentials* creds);

    SyscallError close(InodeFile* fd);

    SyscallError link(const Path& path, const Path& newpath, Credentials* creds);

    SyscallError unlink(const Path& path, Credentials* creds);

    SyscallError getdirentcount(const Path& path, u32* count_out, Credentials* creds);

    SyscallError getdirents(const Path& path, DirEntry* dirents_out, Credentials* creds);

    SyscallError stat(const Path& path, FSStat* stat_out, Credentials* creds);

private:
    Mount* get_mount_at_inode(Inode* inode);
    
    Mount* get_mount_with_root(Inode* inode);

    SyscallError get_inode(Inode* inode_out, const Path& path, Credentials* creds);

    SyscallError create_inode(Inode* inode_out, const Path& path, u32 mode, Credentials* creds);

    SyscallError status_to_syscallerror(FSStatus status) const;

    FileSystem* create_fs_from_fd(FileDescription* fd, const Path& srcpath);

    void init_internal();

public:
    static void init();

    static FileSystemManager* get();

private:
    Mount* rootmount = nullptr;

    Vec<Mount*> mounts;

    usize dev_counter = 0;
};