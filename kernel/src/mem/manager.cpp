#include "manager.hpp"
#include "paging.hpp"

#include <logger.hpp>

MemoryManager MemoryManager::instance;

static u64 kernel_end_page_counter;

MemorySectionManager::MemorySectionManager()
    : range{0, 0} {}

MemorySectionManager::MemorySectionManager(PageRange range)
    : range(range)
{
    init_bitmap();
}

void MemorySectionManager::reserve_page(u64 page) {
    if (!range.page_in_range(page))
        return;
    
    u64 idx8 = page / 8;
    u8  idx  = page % 8;

    if (bitmap[idx8] & idx)
        return;

    bitmap[idx8] |= 1 << idx;
    pages_used++;
}

u64 MemorySectionManager::alloc_page() {
    if (pages_used >= range.page_count())
        return 0;
    
    u64 page_count = range.page_count();

    u64 idx8 = 0;
    for (; idx8 < bitmap_size; idx8++)
        if (bitmap[idx8] != 0xff) break;
    
    if (bitmap[idx8] == 0xff)
        return 0;

    u8 idx = 0;
    for (; idx < 8; idx++) {
        if ((bitmap[idx8] & (1 << idx)) == 0)
            break;
    }

    u64 ret = idx8 * 8 + idx;

    bitmap[idx8] |= 1 << idx;

    pages_used++;
    return range.base + ret;
}

void MemorySectionManager::dealloc_page(u64 page) {
    if (!range.page_in_range(page))
        return;
    
    u64 idx8 = page / 8;
    u8  idx  = page % 8;

    if (bitmap[idx8] & idx)
        return;

    bitmap[idx8] &= ~(1 << idx);

    pages_used--;
}

void MemorySectionManager::init_bitmap() {
    u64 page_count = range.page_count();

    bitmap_size = page_count / 8;
    if (page_count % 8)
        bitmap_size++;
    
    u64 bitmap_size_pages = bitmap_size / ARCH_PAGE_SIZE;
    if (bitmap_size % ARCH_PAGE_SIZE)
        bitmap_size_pages++;

    for (u64 i = 0; i < bitmap_size_pages; i++)
        VirtualMemory::get_kernel_memory()->map_page(kernel_end_page_counter + i, range.base, VMEM_PAGE_READ | VMEM_PAGE_WRITE);

    bitmap = (u8*)(kernel_end_page_counter * ARCH_PAGE_SIZE);
    kernel_end_page_counter += bitmap_size_pages;

    for (u64 i = 0; i < bitmap_size_pages * ARCH_PAGE_SIZE; i++) {
        if (i < page_count / 8) {
            bitmap[i] = 0;
            continue;
        }

        if (i > page_count / 8) {
            bitmap[i] = 0xff;
            continue;
        }

        bitmap[i] = 0;
        for (u8 j = 0; j < (page_count % 8); j++)
            bitmap[i] |= 1 << j;
    }

    for (u64 i = 0; i < bitmap_size_pages; i++)
        alloc_page();
}

MemoryManager::MemoryManager()
    : section_count(0), kheap(0, 0) {}

u64 MemoryManager::total_available_memory() const {
    u64 size = 0;

    for (u32 i = 0; i < section_count; i++)
        size += sections[i].get_range().size();

    return size;
}

void MemoryManager::reserve_page(u64 idx) {
    MemorySectionManager* msm = get_section_at(idx);
    if (msm == nullptr)
        return;

    msm->reserve_page(idx);
}

u64 MemoryManager::alloc_page() {
    for (u32 i = 0; i < section_count; i++) {
        u64 idx = sections[i].alloc_page();
        if (idx != 0)
            return idx;
    }

    panic("Out of memory!");
    return 0;
}

void MemoryManager::dealloc_page(u64 idx) {
    MemorySectionManager* msm = get_section_at(idx);
    if (msm == nullptr)
        return;

    msm->dealloc_page(idx);
}

bool MemoryManager::resize_heap(usize limit) {
    u64 old_pidx = kheap_limit / ARCH_PAGE_SIZE;
    if (kheap_limit % ARCH_PAGE_SIZE) old_pidx++;
    
    u64 new_pidx = limit / ARCH_PAGE_SIZE;
    if (limit % ARCH_PAGE_SIZE) new_pidx++;

    kheap_limit = limit;
    if (new_pidx == old_pidx)
        return true;

    VirtualMemory* kmem = VirtualMemory::get_kernel_memory();

    if (new_pidx < old_pidx) {
        for (u64 i = new_pidx; i < old_pidx; i++)
            kmem->dealloc_page(i);

        return true;
    }
    
    for (u64 i = old_pidx; i < new_pidx; i++) {
        if (!kmem->alloc_page(i, VMEM_PAGE_READ | VMEM_PAGE_WRITE))
            return false;
    }

    return true;
}

static bool heap_resize_func(void* new_limit) {
    return MemoryManager::get()->resize_heap((usize)new_limit);
}

void MemoryManager::init_heap() {
    kheap_base  = kernel_end_page_counter * ARCH_PAGE_SIZE;
    kheap_limit = kheap_base;

    kheap.set_resize_func(heap_resize_func);

    has_initialized = true;
}

usize MemoryManager::kheap_size() const {
    return kheap.get_total_size();
}

void* MemoryManager::kmalloc(usize size, u8 align) {
    return kheap.malloc(size, align);
}

void MemoryManager::kfree(void* ptr) {
    return kheap.free(ptr);
}

void* MemoryManager::krealloc(void* ptr, usize size) {
    return kheap.realloc(ptr, size);
}

bool MemoryManager::has_init() {
    return has_initialized;
}

MemorySectionManager* MemoryManager::get_section_at(u64 idx) {
    for (u32 i = 0; i < section_count; i++) {
        if (sections[i].get_range().page_in_range(idx))
            return &sections[i];
    }

    return nullptr;
}

MemoryManager* MemoryManager::get() {
    return &instance;
}

bool MemoryManager::init() {
    kernel_end_page_counter = VirtualMemoryManager::get_kernel_range().to_pagerange().limit + 1;

    for (auto& prange : PhysicalMemoryMap::get_ranges()) {
        if (instance.section_count >= MEMORY_MAP_MAX_RANGE_COUNT) {
            Log::WARN() << "MemoryManager: Memory ranges exceeds MEMORY_MAP_MAX_RANGE_COUNT";
            return true;
        }

        if (prange.type != PhysicalMemoryType::AVAILABLE)
            continue;

        instance.sections[instance.section_count++] = MemorySectionManager(prange.range.to_inner_pagerange());
    }

    return true;
}