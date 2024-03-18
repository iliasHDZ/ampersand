#pragma once

#include "fs.hpp"

#include <fd/block_transfer.hpp>

#define EXT2_SIGNATURE 0xef53

#define EXT2_BLOCK 0
#define EXT2_INODE 1

enum class ext2_os_id {
    LINUX     = 0,
    GNU_HURD  = 1,
    MASIX     = 2,
    FREEBSD   = 3,
    BSD_LITE  = 4,
    AMPERSAND = 0xade0
};

struct PACKED_STRUCT ext2_superblock {
    u32 num_inodes;
    u32 num_blocks;
    u32 num_reserved_blocks;
    u32 num_free_blocks;
    u32 num_free_inodes;
    u32 superblock_blknum;
    u32 block_size_bits;
    u32 frag_size_bits;
    u32 num_blocks_per_group;
    u32 num_frags_per_group;
    u32 num_inodes_per_group;
    time32 last_mount_time;
    time32 last_write_time;
    u16 mount_count_since_check;
    u16 mount_count_until_check;
    u16 signature;
    u16 fs_state;
    u16 error_action;
    u16 minor_version;
    time32 last_check_time;
    time32 check_time_interval;
    ext2_os_id formatter_os;
    u32 major_version;
    u16 reserved_uid;
    u16 reserved_gid;

    // Extended Superblock (version >= 1.0)
    u32 first_inode_num;
    u16 inode_entry_size;
    u16 sblk_block_group;
    u32 opt_features;
    u32 req_features;
    u32 ro_features;
    u8 filesystem_id[16];
    char volume_name[16];
    char last_mount[64];
    u32 compression;
    u8 file_blocks_prealloc;
    u8 dir_blocks_prealloc;
    u16 __unused;
    u8 journal_id[16];
    u32 journal_inode;
    u32 journal_device;
    u32 orphan_inode_head;
};

struct PACKED_STRUCT ext2_bgd_table_entry {
    u32 block_bitmap_blknum;
    u32 inode_bitmap_blknum;
    u32 inode_table_blknum;
    u16 num_free_blocks;
    u16 num_free_inodes;
    u16 num_dirs;
    u16 __unused[7];
};

struct PACKED_STRUCT ext2_dir_entry {
    u32 inode_num;
    u16 size;
    u8 name_len;
    u8 type;
    char name[0];

    inline ext2_dir_entry* next() {
        return (ext2_dir_entry*)((usize)this + size);
    }

    inline bool name_equals(const char* oname) const {
        for (usize i = 0; i < name_len; i++) {
            if (name[i] != oname[i])
                return false;
        }

        return oname[name_len] == 0;
    }
};

struct ext2_inode_entry;
class Ext2FileDescription;

struct Ext2DeallocResponse {
    u32 blocks_deallocated;
    bool may_dealloc_ptrblk;
};

class Ext2FileSystem : public FileSystem {
public:
    Ext2FileSystem(FileDescription* fd, const char* srcpath);

    ~Ext2FileSystem();

    FileDescription* create_fd(Inode* file) override;

    void free_fd(FileDescription* fd) override;

    FSFileOpenMethod get_open_method(Inode* file) override;

    FSStatus get_root(Inode* root_out) override;

    FSStatus fetch(Inode* directory, Inode* child_out, const char* name) override;

    FSStatus create(Inode* inode_out, u32 mode, UserID uid, GroupID gid) override;

    FSStatus link(Inode* directory, Inode* inode, const char* name) override;

    FSStatus unlink(Inode* directory, const char* name) override;

    FSStatus mkdir(Inode* parent, Inode* dir_out, u32 mode, UserID uid, GroupID gid, const char* name) override;

    // RMDIR HAS NOT BEEN TESTED!
    FSStatus rmdir(Inode* parent, const char* name) override;

    FSStatus getdirentcount(Inode* directory, u32* count_out) override;

    FSStatus getdirents(Inode* directory, DirEntry* dirents_out) override;

    void unmount() override;

    const char* source_path() override;

private:
    bool read_blocks(void* buffer, u32 blknum, u32 count);
    
    bool write_blocks(void* buffer, u32 blknum, u32 count);

    bool read(void* buffer, u64 offset, u64 size);

    bool write(void* buffer, u64 offset, u64 size);

    ext2_dir_entry* dirent_table_fetch(u8* table, const char* name);

    ext2_dir_entry* dirent_table_alloc(u8* table, usize name_len);

    u64 inode_entry_offset(u32 inode_num);

    bool read_inode_entry(ext2_inode_entry* entry, u32 inode_num);

    bool write_inode_entry(ext2_inode_entry* entry, u32 inode_num);

    u32 read_blknum_from_table(u32 blknum, u32 index);

    u32 get_inode_blknum(ext2_inode_entry* entry, u32 block_offset);

    u32 access_inode_blocks(bool write, void* buffer, ext2_inode_entry* entry, u32 block_offset, u32 block_count);

    u32 read_inode_blocks(void* buffer, ext2_inode_entry* entry, u32 block_offset, u32 block_count);

    u32 write_inode_blocks(void* buffer, ext2_inode_entry* entry, u32 block_offset, u32 block_count);

    u32 alloc(u8 type);

    void dealloc(u8 type, u32 num);

    void clear_block(u32 blknum);

    u32 alloc_inode_ptrblk(u32 blknum, u32 count, u32 layer);

    Ext2DeallocResponse dealloc_inode_ptrblk(u32 blknum, u32 count, u32 layer);

    u32 alloc_inode_blocks(ext2_inode_entry* entry, u32 count);

    u32 dealloc_inode_blocks(ext2_inode_entry* entry, u32 count);

    u64 resize_inode(ext2_inode_entry* entry, u64 size);

    u32 create_inode(ext2_inode_entry* entry_out, u32 mode, UserID uid, GroupID gid);

    void remove_inode(ext2_inode_entry* entry, u32 inode_num);

    void inode_entry_to_inode(ext2_inode_entry* entry, Inode* out, u32 inode_num);

    void access_bitmap(bool write, u8 type);

    bool init();

public:
    static bool is_of_type(FileDescription* fd);

private:
    FileDescription* fd;
    char* srcpath;

    u32 blksize_bits;
    u32 block_size;

    u32 group_count;

    usize inode_entry_size;

    ext2_bgd_table_entry* bgd_table = nullptr;

    ext2_superblock sblk;

    u8* block_usage_bitmap = nullptr;
    u8* inode_usage_bitmap = nullptr;

    friend struct ext2_inode_entry;
    friend class Ext2FileDescription;
};

struct PACKED_STRUCT ext2_inode_entry {
    u16 mode;
    UserID uid;
    u32 size;
    time32 access_time;
    time32 create_time;
    time32 modify_time;
    time32 delete_time;
    GroupID gid;
    u16 link_count;
    u32 size_sectors;
    u32 flags;
    u32 osd1;
    u32 direct_blknum[12];
    u32 single_indirect_blknum;
    u32 double_indirect_blknum;
    u32 triple_indirect_blknum;
    u32 generation;
    u32 file_acl;
    u32 dir_acl;
    u32 frag_blknum;
    u32 osd2[3];

    inline u32 block_count(Ext2FileSystem* fs) {
        return divceil(size, fs->block_size);
    }
};

// FIXME: The ext2_inode_entry in the file description does not sync up with the one that would be returned
//        by read_inode_entry in the filesystem!
class Ext2FileDescription : public BlockTransferFileDescription<FileDescription> {
public:
    inline Ext2FileDescription(Ext2FileSystem* fs, u32 inode_num)
        : filesystem(fs), inode_num(inode_num) {}

    bool init();

    bool save();

    u8 get_block_size_bits() const override;

    u64 get_size() override;

    u64 set_size(u64 size) override;

    u64 read_blocks(void* buf, u64 block_offset, u64 block_count) override;

    u64 write_blocks(void* buf, u64 block_offset, u64 block_count) override;

private:
    Ext2FileSystem* filesystem;
    u32 inode_num;
    ext2_inode_entry entry;
};