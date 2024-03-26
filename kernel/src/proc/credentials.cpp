#include "credentials.hpp"
#include <fs/fs.hpp>

Credentials::Credentials(UserID uid, GroupID pgid)
    : uid(uid), pgid(pgid) {}

UserID Credentials::get_uid() const {
    return uid;
}

GroupID Credentials::get_pgid() const {
    return pgid;
}

bool Credentials::in_group(GroupID gid) const {
    return gid == pgid;
}

bool Credentials::may_read(Inode* inode) const {
    return (get_inode_perms(inode) & 4) ? true : false;
}

bool Credentials::may_write(Inode* inode) const {
    return (get_inode_perms(inode) & 2) ? true : false;
}

bool Credentials::may_exec(Inode* inode) const {
    return (get_inode_perms(inode) & 1) ? true : false;
}

u32 Credentials::get_inode_perms(Inode* inode) const {
    if (inode->uid == uid)
        return (inode->mode >> 6) & 0x7;

    if (in_group(inode->gid))
        return (inode->mode >> 3) & 0x7;

    return inode->mode & 0x7;
}