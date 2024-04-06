#pragma once

#include <common.h>
#include <common/vec.hpp>

typedef u16 UserID;
typedef u16 GroupID;

class Inode;

class Credentials {
public:
    Credentials(UserID uid, GroupID pgid);

    UserID get_uid() const;

    GroupID get_pgid() const;

    inline bool is_superuser() const {
        return uid == 0;
    }

    bool in_group(GroupID gid) const;

    bool may_read(Inode* inode) const;

    bool may_write(Inode* inode) const;

    bool may_exec(Inode* inode) const;

private:
    u32 get_inode_perms(Inode* inode) const;

private:
    UserID uid;
    GroupID pgid;
};