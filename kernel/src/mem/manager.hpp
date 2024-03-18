#include "memory.hpp"
#include "heap.hpp"

class MemorySectionManager {
public:
    MemorySectionManager();

    MemorySectionManager(PageRange range);

    inline PageRange get_range() const { return range; };

    inline u64 get_pages_used() const { return pages_used; };

    void reserve_page(u64 idx);

    u64 alloc_page();

    void dealloc_page(u64 page);

private:
    void init_bitmap();

private:
    PageRange range;

    u64 pages_used;

    u64 bitmap_size;
    u8* bitmap;
};

class MemoryManager {
public:
    MemoryManager();

    u64 total_available_memory() const;

    void reserve_page(u64 idx);

    u64 alloc_page();

    void dealloc_page(u64 idx);

    bool resize_heap(usize limit);

    void init_heap();

    usize kheap_size() const;

    void* kmalloc(usize size, u8 align = HEAP_BYTE_ALIGN_BITS);

    void kfree(void* ptr);

    void* krealloc(void* ptr, usize size);

    bool has_init();

    MemorySectionManager* get_section_at(u64 idx);

public:
    static MemoryManager* get();

    static bool init();

private:
    MemorySectionManager sections[MEMORY_MAP_MAX_RANGE_COUNT];

    bool has_initialized = false;

    usize section_count;

    usize kheap_base;
    usize kheap_limit;

    LCHeap kheap;

private:
    static MemoryManager instance;

};