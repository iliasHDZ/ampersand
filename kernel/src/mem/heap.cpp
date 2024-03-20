#include "heap.hpp"

static inline LCHeapBlock* LCHeap_block_after(usize base, u32 align_bits) {
    return (LCHeapBlock*)(alignceil<usize>(base + sizeof(LCHeapBlock), align_bits) - sizeof(LCHeapBlock));
}

void Heap::set_resize_func(HeapResizeRequest func) {
    request_resize = func;
}

LCHeap::LCHeap(void* base, void* limit)
    : base(base), limit(limit), total_size(0)
{
    if (base < (void*)4)
        this->base = (void*)4;

    head = nullptr;
}

void* LCHeap::get_base() const {
    return base;
}

void* LCHeap::get_limit() const {
    return limit;
}

usize LCHeap::get_total_size() const {
    return total_size;
}

usize LCHeap::count_blocks() const {
    LCHeapBlock* block = head;
    usize count = 0;

    while (block) {
        count++;
        block = block->next;
    }

    return count;
}

usize LCHeap::count_total_size() const {
    LCHeapBlock* block = head;
    usize size = 0;

    while (block) {
        size += block->size;
        block = block->next;
    }

    return size;
}

usize LCHeap::get_block_size(void* ptr) {
    LCHeapBlock* block = (LCHeapBlock*)((usize)ptr - sizeof(LCHeapBlock));

    if (!prev_block(block) || head != block)
        return 0;

    return block->size;
}

void* LCHeap::malloc(usize size, u32 align) {
    LCHeapBlock* new_block = LCHeap_block_after((usize)base, align);
    
    LCHeapBlock* block = head;
    LCHeapBlock* prev  = nullptr;

    while (block) {
        if (new_block->fits_before(block, size)) {
            new_block->setup(size, align);
            new_block->next = block;

            if (prev)
                prev->next = new_block;
            else
                head = new_block;

            total_size += size;
            return new_block->data();
        }

        new_block = LCHeap_block_after(block->limit(), align);
        prev  = block;
        block = block->next;
    }

    if (!new_block->fits_with_size(size, (usize)limit)) {
        if (!request_resize || !request_resize((void*)(new_block->data() + size)))
            return nullptr;
    }

    new_block->setup(size, align);
    
    if (prev)
        prev->next = new_block;
    else
        head = new_block;

    total_size += size;
    return new_block->data();
}

void* LCHeap::realloc(void* ptr, usize size) {
    LCHeapBlock* block = (LCHeapBlock*)((usize)ptr - sizeof(LCHeapBlock));
    LCHeapBlock* prev  = prev_block(block);

    if (prev == nullptr && head != block)
        return nullptr;

    usize prev_size = block->size;
    
    if (block->fits_with_size(size, block->next ? block->next->base() : (usize)limit)) {
        total_size -= block->size - size;
        block->size = size;
        return ptr;
    }

    total_size -= block->size;
    prev->next = block->next;

    if (block->next == nullptr && request_resize)
        request_resize((void*)prev->limit());

    void* nptr = malloc(size, block->align_bits);
    if (!nptr) return nullptr;

    memcpy(nptr, ptr, prev_size);
    return nptr;
}

void LCHeap::free(void* ptr) {
    LCHeapBlock* block = (LCHeapBlock*)((usize)ptr - sizeof(LCHeapBlock));
    LCHeapBlock* prev  = prev_block(block);

    if (prev == nullptr && head != block) {
        panic("LCHeap: Attempting to deallocate a non-existant memory block!");
        return;
    }

    total_size -= block->size;

    if (prev == nullptr)
        head = block->next;
    else
        prev->next = block->next;

    if (block->next == nullptr && request_resize)
        request_resize((void*)prev->limit());
}

void LCHeap::set_range(void* base, void* limit) {
    this->base  = base;
    this->limit = limit;
}

LCHeapBlock* LCHeap::prev_block(LCHeapBlock* block) {
    LCHeapBlock* item = head;

    while (item) {
        if (item->next == block)
            return item;

        if (item > block)
            break;
        
        item = item->next;
    }

    return nullptr;
}
