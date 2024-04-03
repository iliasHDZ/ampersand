#pragma once

typedef unsigned char      u8;
typedef unsigned short     u16;
typedef unsigned int       u32;
typedef unsigned long long u64;

typedef char      i8;
typedef short     i16;
typedef int       i32;
typedef long long i64;

typedef i32 isize;
typedef u32 usize;

#define HEAP_BYTE_ALIGN_BITS 2

typedef bool(*HeapResizeRequest)(void* new_limit);

class Heap {
public:
    virtual void* get_base() const = 0;

    virtual void* get_limit() const = 0;

    virtual usize get_total_size() const = 0;

    virtual usize count_blocks() const = 0;

    virtual usize count_total_size() const = 0;

    virtual usize get_block_size(void* ptr) = 0;

    virtual void* malloc(usize size, u32 align_bits = HEAP_BYTE_ALIGN_BITS) = 0;

    virtual void* realloc(void* ptr, usize size) = 0;

    virtual void free(void* ptr) = 0;

public:
    void set_resize_func(HeapResizeRequest func);

protected:
    HeapResizeRequest request_resize = nullptr;

};

struct LCHeapBlock {
    LCHeapBlock* next;
    usize size;
    u32 align_bits;

    inline void* data() {
        return (void*)((usize)this + sizeof(LCHeapBlock));
    }

    inline usize base() {
        return (usize)this;
    }

    inline usize limit() {
        return (usize)data() + size;
    }

    inline bool fits_with_size(usize size, usize limit) {
        if (limit < (usize)data())
            return false;

        return (limit - (usize)data()) >= size;
    }

    inline bool fits_before(LCHeapBlock* next, usize size) {
        return fits_with_size(size, next->base());
    }

    inline void setup(usize size, usize align) {
        next = nullptr;
        this->size = size;
        align_bits = align;
    }
};

struct NewLCHeapBlock {
    LCHeapBlock* new_block;
    LCHeapBlock* prev_block;
};

class LCHeap : public Heap {
public:
    LCHeap(void* base, void* limit);

public:
    void* get_base() const override;

    void* get_limit() const override;

    usize get_total_size() const override;

    usize count_blocks() const override;

    usize count_total_size() const override;

    usize get_block_size(void* ptr) override;

    void* malloc(usize size, u32 align_bits = HEAP_BYTE_ALIGN_BITS) override;

    void* realloc(void* ptr, usize size) override;

    void free(void* ptr) override;

    void set_range(void* base, void* limit);

private:
    LCHeapBlock* prev_block(LCHeapBlock* block);

private:
    LCHeapBlock* head;

    void* base;
    void* limit;

    usize total_size;

};
