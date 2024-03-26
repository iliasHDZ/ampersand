#pragma once

#include <common.h>

#include <fd/fd.hpp>
#include <proc/credentials.hpp>
#include <sys/stat.h>

enum class FSStatus {
    SUCCESS,
    NO_ENTRY,
    NO_SPACE,
    EXISTS,
    NOT_ALLOWED,
    NOT_EMPTY,
    NOT_DIR
};

/*
    This contains the method as to how to open a file in a filesystem or if it should generate and error.

    USE_FILE_DESCRIPTION:
        Use the create_fd function to create a file description of the file. This is
        great for block devices since it allows ioctls.
    NAMED_PIPE:
        This means that the file is a named pipe and doesn't actually need anything
        from the filesystem since they are handled by the FileSystemManager.
*/
enum class FSFileOpenMethod {
    USE_FILE_DESCRIPTION,
    NAMED_PIPE,

    ERR_EXIST,
    ERR_GENERAL,
    ERR_ACCESS
};

struct Inode;
struct DirEntry;

class FileSystem {
public:
    inline FileSystem(const char* name) {
        this->name = name;
    }

    inline const char* get_name() const {
        return name;
    }

    inline usize get_dev_no() const {
        return dev_no;
    }

    inline void set_dev_no(usize dev_no) {
        this->dev_no = dev_no;
    }

    virtual InodeFile* create_fd(Inode* file) = 0;

    virtual void free_fd(InodeFile* fd) = 0;

    virtual FSFileOpenMethod get_open_method(Inode* file) = 0;

    virtual FSStatus get_root(Inode* root_out) = 0;

    virtual FSStatus fetch(Inode* directory, Inode* child_out, const char* name) = 0;

    virtual FSStatus create(Inode* inode_out, u32 mode, UserID uid, GroupID gid) = 0;

    virtual FSStatus link(Inode* directory, Inode* inode, const char* name) = 0;

    virtual FSStatus unlink(Inode* directory, const char* name) = 0;

    virtual FSStatus mkdir(Inode* parent, Inode* dir_out, u32 mode, UserID uid, GroupID gid, const char* name) = 0;

    virtual FSStatus rmdir(Inode* parent, const char* name) = 0;

    virtual FSStatus getdirentcount(Inode* directory, u32* count_out) = 0;

    virtual FSStatus getdirents(Inode* directory, DirEntry* dirents_out) = 0;

    virtual const char* source_path() = 0;

    virtual void unmount() = 0;

private:
    usize dev_no;

    const char* name;
};

struct Inode {
    u32 inode_id;
    FileSystem* filesystem;
    u32 links;
    u64 size;

    u32 mode;
    UserID uid;
    GroupID gid;

    inline bool operator==(const Inode& inode) const {
        return inode_id == inode.inode_id && filesystem == inode.filesystem;
    }

    inline FSStatus fetch(Inode* out, const char* name) {
        if (filesystem == nullptr)
            return FSStatus::NO_ENTRY;

        Inode inode;
        FSStatus status = filesystem->fetch(this, &inode, name);

        if (status == FSStatus::SUCCESS)
            *out = inode;

        return status;
    }
};

struct DirEntry {
    Inode inode;
    char name[256];

    void set_name(const char* name, isize len = -1);
};