#include "ext2.hpp"

#include <logger.hpp>

Ext2FileSystem::Ext2FileSystem(FileDescription* fd, const char* srcpath)
    : FileSystem("ext2"), fd(fd)
{
    if (srcpath != nullptr) {
        usize len = strlen(srcpath);
        this->srcpath = new char[len + 1];
        memcpy(this->srcpath, srcpath, len + 1);
    } else
        this->srcpath = nullptr;

    init();
}

Ext2FileSystem::~Ext2FileSystem() {
    if (srcpath != nullptr)
        delete srcpath;

    if (bgd_table != nullptr)
        delete bgd_table;

    if (block_usage_bitmap != nullptr)
        delete block_usage_bitmap;

    if (inode_usage_bitmap != nullptr)
        delete inode_usage_bitmap;
}

FileDescription* Ext2FileSystem::create_fd(Inode* file) {
    Ext2FileDescription* fd = new Ext2FileDescription(this, file->inode_id);
    if (!fd->init()) {
        delete fd;
        return nullptr;
    }

    return fd;
}

void Ext2FileSystem::free_fd(FileDescription* fd) {
    Ext2FileDescription* ext2fd = (Ext2FileDescription*)fd;

    ext2fd->save();
    delete ext2fd;
}

FSFileOpenMethod Ext2FileSystem::get_open_method(Inode* file) {
    if (!S_ISREG(file->mode))
        return FSFileOpenMethod::ERR_GENERAL;

    return FSFileOpenMethod::USE_FILE_DESCRIPTION;
}

FSStatus Ext2FileSystem::get_root(Inode* root_out) {
    ext2_inode_entry entry;

    if (!read_inode_entry(&entry, 2))
        return FSStatus::NOT_ALLOWED;
    
    inode_entry_to_inode(&entry, root_out, 2);
    return FSStatus::SUCCESS;
}

FSStatus Ext2FileSystem::fetch(Inode* dir, Inode* child_out, const char* name) {
    ext2_inode_entry entry;

    if (!read_inode_entry(&entry, dir->inode_id))
        return FSStatus::NOT_ALLOWED;
    
    usize block_count = divceil(entry.size, block_size);

    u8* block = new u8[block_size];

    for (usize i = 0; i < block_count; i++) {
        if (!read_inode_blocks((void*)block, &entry, i, 1)) {
            delete[] block;
            return FSStatus::NOT_ALLOWED;
        }

        ext2_dir_entry* dirent = dirent_table_fetch(block, name);

        if (dirent) {
            usize inode_num = dirent->inode_num;
            delete[] block;

            if (!read_inode_entry(&entry, inode_num))
                return FSStatus::NOT_ALLOWED;
            
            inode_entry_to_inode(&entry, child_out, dirent->inode_num);
            return FSStatus::SUCCESS;
        }
    }

    delete[] block;
    return FSStatus::NO_ENTRY;
}

FSStatus Ext2FileSystem::create(Inode* inode_out, u32 mode, UserID uid, GroupID gid) {
    ext2_inode_entry entry;
    
    u32 inode_num = create_inode(&entry, mode, uid, gid);
    if (inode_num == 0)
        return FSStatus::NO_SPACE;
    
    inode_entry_to_inode(&entry, inode_out, inode_num);
    return FSStatus::SUCCESS;
}

FSStatus Ext2FileSystem::link(Inode* dir, Inode* inode, const char* name) {
    ext2_inode_entry entry;

    if (!read_inode_entry(&entry, dir->inode_id))
        return FSStatus::NOT_ALLOWED;
    
    usize block_count = divceil(entry.size, block_size);

    u8* block = new u8[block_size];

    u32 len = strlen(name);

    u8 direnttype = 0;

    switch (inode->mode & S_IFMT) {
    case S_IFREG:  direnttype = 1; break;
    case S_IFDIR:  direnttype = 2; break;
    case S_IFCHR:  direnttype = 3; break;
    case S_IFBLK:  direnttype = 4; break;
    case S_IFIFO:  direnttype = 5; break;
    case S_IFSOCK: direnttype = 6; break;
    case S_IFLNK:  direnttype = 7; break;
    default:
        direnttype = 0;
    }

    for (usize i = 0; i < block_count; i++) {
        if (!read_inode_blocks((void*)block, &entry, i, 1)) {
            delete[] block;
            return FSStatus::NOT_ALLOWED;
        }

        ext2_dir_entry* dirent = dirent_table_alloc(block, len);

        if (dirent) {
            dirent->inode_num = inode->inode_id;
            dirent->name_len  = len;
            dirent->type      = direnttype;
            memcpy(dirent->name, name, len);

            if (!write_inode_blocks((void*)block, &entry, i, 1)) {
                delete[] block;
                return FSStatus::NOT_ALLOWED;
            }

            if (!write_inode_entry(&entry, dir->inode_id))
                return FSStatus::NOT_ALLOWED;

            if (!read_inode_entry(&entry, inode->inode_id))
                return FSStatus::NOT_ALLOWED;

            entry.link_count++;

            if (!write_inode_entry(&entry, inode->inode_id))
                return FSStatus::NOT_ALLOWED;

            delete[] block;
            return FSStatus::SUCCESS;
        }
    }

    u64 nsize = (block_count + 1) << blksize_bits;

    if (resize_inode(&entry, nsize) != nsize)
        return FSStatus::NO_SPACE;
    
    if (!read_inode_blocks((void*)block, &entry, block_count, 1)) {
        delete[] block;
        return FSStatus::NOT_ALLOWED;
    }

    ext2_dir_entry* dirent = (ext2_dir_entry*)block;

    dirent->inode_num = inode->inode_id;
    dirent->name_len  = len;

    dirent->type = direnttype;
    dirent->size = block_size;

    memcpy(dirent->name, name, len);
    
    if (!write_inode_blocks((void*)block, &entry, block_count, 1)) {
        delete[] block;
        return FSStatus::NOT_ALLOWED;
    }

    if (!write_inode_entry(&entry, dir->inode_id))
        return FSStatus::NOT_ALLOWED;

    if (!read_inode_entry(&entry, inode->inode_id))
        return FSStatus::NOT_ALLOWED;

    entry.link_count++;

    if (!write_inode_entry(&entry, inode->inode_id))
        return FSStatus::NOT_ALLOWED;

    delete[] block;
    return FSStatus::SUCCESS;
}

FSStatus Ext2FileSystem::unlink(Inode* dir, const char* name) {
    ext2_inode_entry entry;

    if (!read_inode_entry(&entry, dir->inode_id))
        return FSStatus::NOT_ALLOWED;
    
    usize block_count = divceil(entry.size, block_size);

    u8* block = new u8[block_size];

    for (usize i = 0; i < block_count; i++) {
        if (!read_inode_blocks((void*)block, &entry, i, 1)) {
            delete[] block;
            return FSStatus::NOT_ALLOWED;
        }

        ext2_dir_entry* dirent = dirent_table_fetch(block, name);

        if (dirent) {
            u32 inode_num = dirent->inode_num;

            dirent->inode_num = 0;
            if (!write_inode_blocks((void*)block, &entry, i, 1)) {
                delete[] block;
                return FSStatus::NOT_ALLOWED;
            }

            if (!read_inode_entry(&entry, inode_num))
                return FSStatus::NOT_ALLOWED;

            entry.link_count--;
            if (entry.link_count == 0)
                remove_inode(&entry, inode_num);

            if (!write_inode_entry(&entry, inode_num))
                return FSStatus::NOT_ALLOWED;

            delete[] block;
            return FSStatus::SUCCESS;
        }
    }

    delete[] block;
    return FSStatus::NO_ENTRY;
}

FSStatus Ext2FileSystem::mkdir(Inode* parent, Inode* dir_out, u32 mode, UserID uid, GroupID gid, const char* name) {
    ext2_inode_entry entry;
    
    u32 inode_num = create_inode(&entry, mode, uid, gid);
    if (inode_num == 0)
        return FSStatus::NO_SPACE;

    inode_entry_to_inode(&entry, dir_out, inode_num);

    FSStatus status = link(parent, dir_out, name);
    if (status != FSStatus::SUCCESS) {
        remove_inode(&entry, inode_num);
        return status;
    }

    status = link(dir_out, dir_out, ".");
    if (status != FSStatus::SUCCESS) {
        remove_inode(&entry, inode_num);
        return status;
    }

    status = link(dir_out, parent, "..");
    if (status != FSStatus::SUCCESS) {
        remove_inode(&entry, inode_num);
        return status;
    }
    
    return FSStatus::SUCCESS;
}

FSStatus Ext2FileSystem::rmdir(Inode* parent, const char* name) {
    Inode dir;

    FSStatus status = fetch(parent, &dir, name);
    if (status != FSStatus::SUCCESS)
        return status;
    
    if (!S_ISDIR(dir.mode))
        return FSStatus::NOT_DIR;

    ext2_inode_entry entry;
    if (!read_inode_entry(&entry, dir.inode_id))
        return FSStatus::NOT_ALLOWED;
    
    usize block_count = divceil(entry.size, block_size);

    u8* block = new u8[block_size];

    ext2_dir_entry* begin = (ext2_dir_entry*)block;
    ext2_dir_entry* end   = (ext2_dir_entry*)(block + block_size);

    for (usize i = 0; i < block_count; i++) {
        if (!read_inode_blocks((void*)block, &entry, i, 1)) {
            delete[] block;
            return FSStatus::NOT_ALLOWED;
        }

        for (ext2_dir_entry* dirent = begin; dirent != end; dirent = dirent->next()) {
            if (dirent->inode_num != 0 && !dirent->name_equals(".") && !dirent->name_equals("..")) {
                delete[] block;
                return FSStatus::NOT_EMPTY;
            }
        }
    }

    delete[] block;

    status = unlink(parent, name);
    if (status != FSStatus::SUCCESS)
        return status;

    status = unlink(&dir, ".");
    if (status != FSStatus::SUCCESS)
        return status;

    status = unlink(&dir, "..");
    if (status != FSStatus::SUCCESS)
        return status;

    remove_inode(&entry, dir.inode_id);
    return FSStatus::SUCCESS;
}

FSStatus Ext2FileSystem::getdirentcount(Inode* dir, u32* count_out) {
    ext2_inode_entry entry;

    if (!read_inode_entry(&entry, dir->inode_id))
        return FSStatus::NOT_ALLOWED;
    
    usize bsize = alignceil(entry.size, block_size);

    u8* buffer = new u8[bsize];

    if (!read_inode_blocks((void*)buffer, &entry, 0, divceil(entry.size, block_size))) {
        delete[] buffer;
        return FSStatus::NOT_ALLOWED;
    }

    ext2_dir_entry* dirent = (ext2_dir_entry*)buffer;
    ext2_dir_entry* end    = (ext2_dir_entry*)(buffer + bsize);

    *count_out = 0;

    for (; dirent != end; dirent = dirent->next())
        (*count_out)++;

    delete[] buffer;
    return FSStatus::SUCCESS;
}

FSStatus Ext2FileSystem::getdirents(Inode* dir, DirEntry* out) {
    ext2_inode_entry entry;

    if (!read_inode_entry(&entry, dir->inode_id))
        return FSStatus::NOT_ALLOWED;
    
    usize bsize = alignceil(entry.size, block_size);

    u8* buffer = new u8[bsize];

    if (!read_inode_blocks((void*)buffer, &entry, 0, divceil(entry.size, block_size))) {
        delete[] buffer;
        return FSStatus::NOT_ALLOWED;
    }

    ext2_dir_entry* dirent = (ext2_dir_entry*)buffer;
    ext2_dir_entry* end    = (ext2_dir_entry*)(buffer + bsize);

    usize idx = 0;
    for (; dirent != end; dirent = dirent->next()) {
        out[idx].set_name(dirent->name, dirent->name_len);
        if (!read_inode_entry(&entry, dirent->inode_num)) {
            delete[] buffer;
            return FSStatus::NOT_ALLOWED;
        }
        
        inode_entry_to_inode(&entry, &out[idx].inode, dirent->inode_num);
        idx++;
    }

    delete[] buffer;
    return FSStatus::SUCCESS;
}

void Ext2FileSystem::unmount() {
    access_bitmap(true, EXT2_BLOCK);
    access_bitmap(true, EXT2_INODE);

    write(&sblk, 0x400, sizeof(ext2_superblock));

    write(bgd_table, (blksize_bits == 10 ? 2 : 1) << blksize_bits, group_count * sizeof(ext2_bgd_table_entry));
}

const char* Ext2FileSystem::source_path() {
    return srcpath;
}

bool Ext2FileSystem::read_blocks(void* buffer, u32 blknum, u32 count) {
    u64 offset = (u64)blknum << blksize_bits;
    u64 size   = (u64)count  << blksize_bits;

    return read(buffer, offset, size);
}

bool Ext2FileSystem::write_blocks(void* buffer, u32 blknum, u32 count) {
    u64 offset = (u64)blknum << blksize_bits;
    u64 size   = (u64)count  << blksize_bits;

    return write(buffer, offset, size);
}

bool Ext2FileSystem::read(void* buffer, u64 offset, u64 size) {
    return fd->read(buffer, offset, size) == size;
}

bool Ext2FileSystem::write(void* buffer, u64 offset, u64 size) {
    return fd->write(buffer, offset, size) == size;
}

ext2_dir_entry* Ext2FileSystem::dirent_table_fetch(u8* table, const char* name) {
    ext2_dir_entry* begin = (ext2_dir_entry*)table;
    ext2_dir_entry* end   = (ext2_dir_entry*)(table + block_size);

    for (ext2_dir_entry* dirent = begin; dirent != end; dirent = dirent->next()) {
        if (dirent->name_equals(name))
            return dirent;
    }

    return nullptr;
}

ext2_dir_entry* Ext2FileSystem::dirent_table_alloc(u8* table, usize name_len) {
    ext2_dir_entry* begin = (ext2_dir_entry*)table;
    ext2_dir_entry* end   = (ext2_dir_entry*)(table + block_size);

    usize size = alignceil<usize>(name_len + sizeof(ext2_dir_entry), 2);

    ext2_dir_entry* start_ent = begin;
    usize size_found = 0;

    for (ext2_dir_entry* dirent = begin; dirent != end; dirent = dirent->next()) {
        if (dirent->inode_num != 0) {
            start_ent  = dirent;
            size_found = dirent->size - alignceil<usize>(sizeof(ext2_dir_entry) + dirent->name_len, 2);
        } else {
            size_found += dirent->size;
        }

        if (size_found >= size)
            break;
    }

    if (size_found < size)
        return nullptr;

    start_ent->size = alignceil<usize>(sizeof(ext2_dir_entry) + start_ent->name_len, 2);

    ext2_dir_entry* ret = (ext2_dir_entry*)((usize)start_ent + start_ent->size);

    ret->size = size_found;

    return ret;
}

u64 Ext2FileSystem::inode_entry_offset(u32 inode_num) {
    u32 block_group = (inode_num - 1) / sblk.num_inodes_per_group;
    u32 index       = (inode_num - 1) % sblk.num_inodes_per_group;

    if (block_group > group_count)
        return 0;

    u32 table_blknum = bgd_table[block_group].inode_table_blknum;

    return ( (u64)table_blknum << blksize_bits ) + index * inode_entry_size;
}

bool Ext2FileSystem::read_inode_entry(ext2_inode_entry* entry, u32 inode_num) {
    u64 offset = inode_entry_offset(inode_num);
    if (offset == 0) return false;

    return read(entry, offset, sizeof(ext2_inode_entry));
}

bool Ext2FileSystem::write_inode_entry(ext2_inode_entry* entry, u32 inode_num) {
    u64 offset = inode_entry_offset(inode_num);
    if (offset == 0) return false;

    return write(entry, offset, sizeof(ext2_inode_entry));
}

u32 Ext2FileSystem::read_blknum_from_table(u32 blknum, u32 index) {
    if (blknum == 0)
        return 0;

    u32* ptrtab = new u32[block_size / 4];
    if (!read_blocks(ptrtab, blknum, 1)) {
        delete[] ptrtab;
        return 0;
    }

    u32 ret = ptrtab[index];
    delete[] ptrtab;
    return ret;
}

u32 Ext2FileSystem::get_inode_blknum(ext2_inode_entry* entry, u32 block_offset) {
    u32 blknum;

    if (block_offset < 12)
        return entry->direct_blknum[block_offset];

    u32 ent_count0 = block_size / 4;
    u32 ent_count1 = ent_count0 * ent_count0;

    block_offset -= 12;

    if (block_offset < ent_count0)
        return read_blknum_from_table(entry->single_indirect_blknum, block_offset);

    block_offset -= ent_count0;

    if (block_offset < ent_count1) {
        u32 idx0 = block_offset / ent_count0;
        u32 idx1 = block_offset % ent_count0;

        blknum = read_blknum_from_table(entry->double_indirect_blknum, idx0);
        if (blknum == 0) return 0;

        return read_blknum_from_table(blknum, idx1);
    }

    block_offset -= ent_count1;

    u32 idx0 = block_offset / ent_count1;
    u32 idx1 = (block_offset - idx0 * ent_count1) / ent_count0;
    u32 idx2 = (block_offset - idx0 * ent_count1) % ent_count0;

    blknum = read_blknum_from_table(entry->triple_indirect_blknum, idx0);
    if (blknum == 0) return 0;

    blknum = read_blknum_from_table(blknum, idx1);
    if (blknum == 0) return 0;

    return read_blknum_from_table(blknum, idx2);
}

u32 Ext2FileSystem::access_inode_blocks(bool write, void* buffer, ext2_inode_entry* entry, u32 block_offset, u32 block_count) {
    u8* buf = (u8*)buffer;

    for (u32 i = 0; i < block_count; i++) {
        u32 blknum = get_inode_blknum(entry, block_offset + i);

        if (blknum == 0)
            return i;
        
        bool success;
        if (write)
            success = write_blocks(buf, blknum, 1);
        else
            success = read_blocks(buf, blknum, 1);

        if (!success) return i;

        buf += block_size;
    }

    return block_count;
}

u32 Ext2FileSystem::read_inode_blocks(void* buffer, ext2_inode_entry* entry, u32 block_offset, u32 block_count) {
    entry->access_time = time();
    return access_inode_blocks(false, buffer, entry, block_offset, block_count);
}

u32 Ext2FileSystem::write_inode_blocks(void* buffer, ext2_inode_entry* entry, u32 block_offset, u32 block_count) {
    entry->modify_time = time();
    return access_inode_blocks(true, buffer, entry, block_offset, block_count);
}

u32 Ext2FileSystem::alloc(u8 type) {
    u8* bitmap;

    if (type == EXT2_BLOCK) {
        if (sblk.num_free_blocks <= 0)
            return 0;

        bitmap = block_usage_bitmap;
    } else {
        if (sblk.num_free_inodes <= 0)
            return 0;

        bitmap = inode_usage_bitmap;
    }

    usize bitmap_size = group_count * block_size;

    u8  bit_idx = 0;
    u32 idx8    = 0;

    for (; idx8 < bitmap_size; idx8++) {
        if (bitmap[idx8] != 0xff)
            break;
    }

    if (idx8 == bitmap_size)
        return 0;

    for (; bit_idx < 8; bit_idx++) {
        if ((bitmap[idx8] & (1 << bit_idx)) == 0)
            break;
    }

    if (bit_idx == 8)
        return 0;
    
    usize block_group = idx8 / block_size;
    usize index       = (idx8 % block_size) * 8 + bit_idx;

    if (index >= sblk.num_blocks_per_group)
        return 0;

    bitmap[idx8] |= 1 << bit_idx;

    u32 ret = block_group * sblk.num_blocks_per_group + index;

    if (type == EXT2_BLOCK) {
        sblk.num_free_blocks--;
        bgd_table[block_group].num_free_blocks--;
        Log::INFO() << "ALLOC BLOCK " << ret << '\n';
    } else {
        sblk.num_free_inodes--;
        bgd_table[block_group].num_free_inodes--;
        Log::INFO() << "ALLOC INODE " << ret << '\n';
        
        ret++;
    }
    
    return ret;
}

void Ext2FileSystem::dealloc(u8 type, u32 num) {
    if (type == EXT2_INODE)
        num--;

    u32 num_per_group = (type == EXT2_BLOCK) ? sblk.num_blocks_per_group : sblk.num_inodes_per_group;
    u8* bitmap        = (type == EXT2_BLOCK) ? block_usage_bitmap : inode_usage_bitmap;

    u32 block_group = num / num_per_group;
    u32 index       = num % num_per_group;
    
    u32 idx8 = block_group * block_size + index / 8;

    u8 bit = 1 << (index % 8);

    if ((bitmap[idx8] & bit) == 0)
        return;

    bitmap[idx8] &= ~bit;

    if (type == EXT2_BLOCK) {
        sblk.num_free_blocks++;
        bgd_table[block_group].num_free_blocks++;
    } else {
        sblk.num_free_inodes++;
        bgd_table[block_group].num_free_inodes++;
    }
}

void Ext2FileSystem::clear_block(u32 blknum) {
    u8* block = new u8[block_size];
    memset(block, 0, block_size);
    write_blocks(block, blknum, 1);
    delete[] block;
}

u32 Ext2FileSystem::alloc_inode_ptrblk(u32 blknum, u32 count, u32 layer) {
    u32* ptrblk = new u32[block_size / 4];
    if (!read_blocks(ptrblk, blknum, 1)) {
        delete[] ptrblk;
        return 0;
    }

    u32 blocks_allocated = 0;
    usize idx = (block_size / 4) - 1;
    for (; idx >= 0; idx--) {
        if (ptrblk[idx] != 0)
            break;

        if (idx == 0)
            break;
    }
    
    for (; idx < block_size / 4; idx++) {
        u32 blkn = ptrblk[idx];
        if (blkn == 0) {
            blkn = alloc(EXT2_BLOCK);
            if (blkn == 0)
                break;

            if (layer > 0)
                clear_block(blkn);
            else
                blocks_allocated++;
            ptrblk[idx] = blkn;
        }

        if (layer > 0)
            blocks_allocated += alloc_inode_ptrblk(blkn, count - blocks_allocated, layer - 1);

        if (blocks_allocated > count)
            panic("Ext2FileSystem: Somehow allocated more blocks than requested");

        if (blocks_allocated == count)
            break;
    }

    if (!write_blocks(ptrblk, blknum, 1)) {
        delete[] ptrblk;
        return 0;
    }

    delete[] ptrblk;
    return blocks_allocated;
}

Ext2DeallocResponse Ext2FileSystem::dealloc_inode_ptrblk(u32 blknum, u32 count, u32 layer) {
    u32* ptrblk = new u32[block_size / 4];
    if (!read_blocks(ptrblk, blknum, 1)) {
        delete[] ptrblk;
        return { 0, false };
    }

    u32 blocks_deallocated = 0;
    isize idx = 0;
    for (; idx < block_size / 4; idx++)
        if (ptrblk[idx] == 0)
            break;
    
    if (idx <= 0)
        return { 0, true };
    
    idx--;
    
    for (; idx >= 0; idx--) {
        u32 blkn = ptrblk[idx];

        if (layer > 0) {
            Ext2DeallocResponse res = dealloc_inode_ptrblk(blkn, count - blocks_deallocated, layer - 1);
            blocks_deallocated += res.blocks_deallocated;
            if (res.may_dealloc_ptrblk) {
                dealloc(EXT2_BLOCK, blkn);
                ptrblk[idx] = 0;
            }
        } else {
            dealloc(EXT2_BLOCK, blkn);
            ptrblk[idx] = 0;
            blocks_deallocated++;
        }

        if (blocks_deallocated > count)
            panic("Ext2FileSystem: Somehow deallocated more blocks than requested");

        if (blocks_deallocated == count)
            break;
    }

    if (!write_blocks(ptrblk, blknum, 1)) {
        delete[] ptrblk;
        return { 0, false };
    }

    delete[] ptrblk;
    return { blocks_deallocated, idx <= 0 };
}

u32 Ext2FileSystem::alloc_inode_blocks(ext2_inode_entry* entry, u32 count) {
    u32 blocks_allocated = 0;

    if (count == 0)
        return 0;

    for (u8 i = 0; i < 12; i++) {
        if (entry->direct_blknum[i] != 0)
            continue;
        
        u32 blknum = alloc(EXT2_BLOCK);
        if (blknum == 0)
            return blocks_allocated;

        entry->direct_blknum[i] = blknum;
        blocks_allocated++;

        if (blocks_allocated >= count)
            return blocks_allocated;
    }

    if (entry->single_indirect_blknum == 0) {
        u32 blknum = alloc(EXT2_BLOCK);
        if (blknum == 0) return blocks_allocated;
        clear_block(blknum);
        entry->single_indirect_blknum = blknum;
    }

    blocks_allocated += alloc_inode_ptrblk(entry->single_indirect_blknum, count - blocks_allocated, 0);

    if (blocks_allocated >= count)
        return blocks_allocated;

    if (entry->double_indirect_blknum == 0) {
        u32 blknum = alloc(EXT2_BLOCK);
        if (blknum == 0) return blocks_allocated;
        clear_block(blknum);
        entry->double_indirect_blknum = blknum;
    }

    blocks_allocated += alloc_inode_ptrblk(entry->double_indirect_blknum, count - blocks_allocated, 1);

    if (blocks_allocated >= count)
        return blocks_allocated;

    if (entry->triple_indirect_blknum == 0) {
        u32 blknum = alloc(EXT2_BLOCK);
        if (blknum == 0) return blocks_allocated;
        clear_block(blknum);
        entry->triple_indirect_blknum = blknum;
    }

    return blocks_allocated + alloc_inode_ptrblk(entry->triple_indirect_blknum, count - blocks_allocated, 2);
}

u32 Ext2FileSystem::dealloc_inode_blocks(ext2_inode_entry* entry, u32 count) {
    u32 blocks_deallocated = 0;
    Ext2DeallocResponse res;

    if (count == 0)
        return 0;

    if (entry->triple_indirect_blknum != 0) {
        res = dealloc_inode_ptrblk(entry->triple_indirect_blknum, count - blocks_deallocated, 2);
        blocks_deallocated += res.blocks_deallocated;
        if (res.may_dealloc_ptrblk) {
            dealloc(EXT2_BLOCK, entry->triple_indirect_blknum);
            entry->triple_indirect_blknum = 0;
        }
    }

    if (blocks_deallocated >= count)
        return blocks_deallocated;

    if (entry->double_indirect_blknum != 0) {
        res = dealloc_inode_ptrblk(entry->double_indirect_blknum, count - blocks_deallocated, 1);
        blocks_deallocated += res.blocks_deallocated;
        if (res.may_dealloc_ptrblk) {
            dealloc(EXT2_BLOCK, entry->double_indirect_blknum);
            entry->double_indirect_blknum = 0;
        }
    }

    if (blocks_deallocated >= count)
        return blocks_deallocated;

    if (entry->single_indirect_blknum != 0) {
        res = dealloc_inode_ptrblk(entry->single_indirect_blknum, count - blocks_deallocated, 0);
        blocks_deallocated += res.blocks_deallocated;
        if (res.may_dealloc_ptrblk) {
            dealloc(EXT2_BLOCK, entry->single_indirect_blknum);
            entry->single_indirect_blknum = 0;
        }
    }

    if (blocks_deallocated >= count)
        return blocks_deallocated;

    for (u8 i = 11; i >= 0; i--) {
        if (entry->direct_blknum[i] == 0)
            continue;
        
        dealloc(EXT2_BLOCK, entry->direct_blknum[i]);
        entry->direct_blknum[i] = 0;

        blocks_deallocated++;

        if (blocks_deallocated >= count)
            return blocks_deallocated;
    }

    return blocks_deallocated;
}

u64 Ext2FileSystem::resize_inode(ext2_inode_entry* entry, u64 size) {
    u64 oldsize = entry->size;
    u64 newsize = size;

    u32 oldsize_blocks = (oldsize >> blksize_bits) + ((oldsize & (block_size - 1)) != 0);
    u32 newsize_blocks = (newsize >> blksize_bits) + ((newsize & (block_size - 1)) != 0);

    if (newsize_blocks < oldsize_blocks)
        dealloc_inode_blocks(entry, oldsize_blocks - newsize_blocks);
    else if (newsize_blocks > oldsize_blocks) {
        u32 blocks_to_allocate = newsize_blocks - oldsize_blocks;

        u32 blocks_allocated = alloc_inode_blocks(entry, blocks_to_allocate);

        if (blocks_allocated < blocks_to_allocate) {
            newsize_blocks = oldsize_blocks + blocks_allocated;
            newsize = newsize_blocks << blksize_bits;
        }
    }

    entry->size = newsize;
    entry->size_sectors = newsize_blocks << (blksize_bits - 9);

    return newsize;
}

u32 Ext2FileSystem::create_inode(ext2_inode_entry* entry, u32 mode, UserID uid, GroupID gid) {
    u32 ino = alloc(EXT2_INODE);

    if (ino == 0)
        return 0;

    read_inode_entry(entry, ino);

    entry->mode = mode;
    entry->uid  = uid;
    entry->gid  = gid;
    entry->size = 0;

    entry->access_time = time();
    entry->create_time = time();
    entry->modify_time = time();
    
    entry->link_count   = 0;
    entry->size_sectors = 0;
    entry->flags        = 0;
    
    memset(entry->direct_blknum, 0, sizeof(entry->direct_blknum));
    entry->single_indirect_blknum = 0;
    entry->double_indirect_blknum = 0;
    entry->triple_indirect_blknum = 0;

    write_inode_entry(entry, ino);
    return ino;
}

void Ext2FileSystem::remove_inode(ext2_inode_entry* entry, u32 inode_num) {
    if (entry->size > 0)
        resize_inode(entry, 0);
    
    entry->mode = 0;
    entry->delete_time = time();

    dealloc(EXT2_INODE, inode_num);
}

void Ext2FileSystem::inode_entry_to_inode(ext2_inode_entry* entry, Inode* out, u32 inode_num) {
    out->inode_id = inode_num;
    out->filesystem = this;
    out->links = entry->link_count;
    out->size = entry->size;

    out->mode = entry->mode;
    out->uid  = entry->uid;
    out->gid  = entry->gid;
}

void Ext2FileSystem::access_bitmap(bool write, u8 type) {
    u8* bitmap = (type == EXT2_BLOCK) ? block_usage_bitmap : inode_usage_bitmap;

    for (u32 i = 0; i < group_count; i++) {
        u32 blknum = (type == EXT2_BLOCK) ? bgd_table[i].block_bitmap_blknum : bgd_table[i].inode_bitmap_blknum;

        if (write)
            write_blocks(bitmap, blknum, 1);
        else
            read_blocks(bitmap, blknum, 1);

        bitmap += block_size;
    }
}

bool Ext2FileSystem::init() {
    if (!read(&sblk, 0x400, sizeof(ext2_superblock)))
        return false;

    Log::INFO("Ext2FileSystem") << Out::dec() << srcpath << ": Mounting ext2 " << sblk.major_version << '.' << sblk.minor_version << "...\n";

    sblk.last_mount_time = time();

    blksize_bits = sblk.block_size_bits + 10;
    block_size   = 1 << blksize_bits;

    inode_entry_size = (sblk.major_version < 1) ? 128 : sblk.inode_entry_size;

    u32 group_count1 = divceil(sblk.num_blocks, sblk.num_blocks_per_group);
    u32 group_count2 = divceil(sblk.num_inodes, sblk.num_inodes_per_group);

    if (group_count1 != group_count2)
        return false;

    group_count = group_count1;

    bgd_table = new ext2_bgd_table_entry[group_count];
    
    usize bgd_table_blknum = blksize_bits == 10 ? 2 : 1;
    usize bgd_table_bytes  = group_count * sizeof(ext2_bgd_table_entry);

    if (!read(bgd_table, bgd_table_blknum << blksize_bits, bgd_table_bytes))
        return false;

    auto& o = Log::INFO("Ext2FileSystem");
    o << "- Block Size: ";
    o.write_size(1 << blksize_bits);
    o << '\n';

    Log::INFO("Ext2FileSystem") << "- Blocks Used: " << (sblk.num_blocks - sblk.num_free_blocks) << " / " << sblk.num_blocks << '\n';
    Log::INFO("Ext2FileSystem") << "- Inodes Used: " << (sblk.num_inodes - sblk.num_free_inodes) << " / " << sblk.num_inodes << '\n';
    Log::INFO("Ext2FileSystem") << "- Block Group Count: " << group_count << '\n';

    block_usage_bitmap = new u8[group_count * block_size];
    inode_usage_bitmap = new u8[group_count * block_size];

    access_bitmap(false, EXT2_BLOCK);
    access_bitmap(false, EXT2_INODE);

    return true;
}

bool Ext2FileSystem::is_of_type(FileDescription* fd) {
    u16 signature;
    u32 v = fd->read(&signature, 0x438, 2);

    if (v != 2)
        return false;

    return signature == EXT2_SIGNATURE;
}

bool Ext2FileDescription::init() {
    if (!filesystem->read_inode_entry(&entry, inode_num))
        return false;
    
    if (!S_ISREG(entry.mode))
        return false;
    
    return true;
}

bool Ext2FileDescription::save() {
    return filesystem->write_inode_entry(&entry, inode_num);
}

u8 Ext2FileDescription::get_block_size_bits() const {
    return filesystem->blksize_bits;
}

u64 Ext2FileDescription::get_size() {
    return entry.size;
}

u64 Ext2FileDescription::set_size(u64 size) {
    return filesystem->resize_inode(&entry, size);
}

u64 Ext2FileDescription::read_blocks(void* buf, u64 block_offset, u64 block_count) {
    return filesystem->read_inode_blocks(buf, &entry, block_offset, block_count);
}

u64 Ext2FileDescription::write_blocks(void* buf, u64 block_offset, u64 block_count) {
    return filesystem->write_inode_blocks(buf, &entry, block_offset, block_count);
}