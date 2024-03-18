#pragma once

#include <common.h>

#define BLOCK_SIZE_128B  7
#define BLOCK_SIZE_256B  8
#define BLOCK_SIZE_512B  9
#define BLOCK_SIZE_1024B 10
#define BLOCK_SIZE_2048B 11

#define ACCESS_READ  0
#define ACCESS_WRITE 1

template <typename T>
class BlockTransferFileDescription : public T {
public:
    virtual ~BlockTransferFileDescription() {
        if (temp_block)
            delete temp_block;
    }

    virtual u8 get_block_size_bits() const = 0;

    virtual u64 read_blocks(void* buf, u64 block_offset, u64 block_count) = 0;

    virtual u64 write_blocks(void* buf, u64 block_offset, u64 block_count) = 0;

    virtual u64 set_size(u64 size) {
        return 0xffffffffffffffff;
    }

    bool has_size() override {
        return true;
    }

    virtual u64 get_size() = 0;

    u64 read(void* out, u64 offset, u64 size) override {
        return access(ACCESS_READ, out, offset, size);
    }

    u64 write(void* in, u64 offset, u64 size) override {
        return access(ACCESS_WRITE, in, offset, size);
    }

private:
    u64 access(u8 dir, void* buffer, u64 offset, u64 size) {
        if (size == 0) return 0;
        check_block_size_changed();

        if (offset + size > get_size()) {
            if (dir == ACCESS_WRITE) {
                u64 nsize = set_size(offset + size);

                if (nsize == 0xffffffffffffffff)
                    size = get_size() - offset;
                else
                    size = min(size, nsize - offset);
            } else {
                size = get_size() - offset;
            }
        }

        u8* ptr = (u8*)buffer;

        u64 limit = offset + size;

        u64 first_block = offset >> block_bits;
        u64 last_block  = limit  >> block_bits;

        // If the base and limit are both in the same block, do a simple block section access.
        if (first_block == last_block) {
            if (access_block_section(dir, ptr, first_block, offset & block_mask, limit & block_mask))
                return size;
            else
                return 0;
        }

        u64 first_aligned_block = first_block;

        // If the base is unaligned, a section of the base block is accessed from the base itself to the end of the block
        if (offset & block_mask) {
            if (!access_block_section(dir, ptr, first_block, offset & block_mask, block_size))
                return 0;
            
            first_aligned_block++;
            ptr += block_size - (offset & block_mask);
        }

        // Access all the aligned blocks
        u64 aligned_blocks_count = last_block - first_aligned_block;
        u64 blocks_accessed = access_blocks(dir, ptr, first_aligned_block, aligned_blocks_count);

        ptr += blocks_accessed * block_size;

        if (blocks_accessed < aligned_blocks_count)
            return (u64)ptr - (u64)buffer;

        // If the limit is unaligned, a section of the limit block is accessed from the begin of the block to the limit itself
        if (limit & block_mask) {
            if (!access_block_section(dir, ptr, last_block, 0, limit & block_mask))
                return (u64)ptr - (u64)buffer;
            
            ptr += block_size - (offset & block_mask);
        }

        return (u64)ptr - (u64)buffer;
    }

    bool access_block_section(u8 dir, u8* buf, u64 block_offset, usize base, usize limit) {
        if (read_blocks(temp_block, block_offset, 1) != 1)
            return false;

        if (dir == ACCESS_READ) {
            memcpy(buf, (void*)(temp_block + base), limit - base);
            return true;
        }

        memcpy((void*)(temp_block + base), buf, limit - base);

        return write_blocks(temp_block, block_offset, 1) == 1;
    }

    u64 access_blocks(u8 dir, u8* buf, u64 block_offset, u64 block_count) {
        if (block_count == 0) return 0;
        if (dir == ACCESS_READ)
            return read_blocks(buf, block_offset, block_count);
        else
            return write_blocks(buf, block_offset, block_count);
    }

    void check_block_size_changed() {
        u8 bbits = get_block_size_bits();
        if (bbits != block_bits) {
            block_bits = bbits;
            block_size = 1 << block_bits;
            block_mask = ~((u64)0xffffffffffffffff << block_bits);

            if (temp_block)
                delete temp_block;

            temp_block = new u8[block_size];
        }
    }

private:
    u8  block_bits = 255;
    u64 block_size = 0;
    u64 block_mask = 0;

    u8* temp_block = nullptr;

};