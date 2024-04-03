#include "memory.hpp"
#include <mem/manager.hpp>
#include <logger.hpp>

MemorySegment::MemorySegment(u64 page_count, VMemPerms perms, bool shared)
    : pages(page_count), perms(perms), shared(shared) {}

MemorySegment* MemorySegment::create(u64 page_count, VMemPerms perms, bool stack, bool shared) {
    MemorySegment* ret = new MemorySegment(page_count, perms, shared);

    ret->stack = stack;
    
    u64 page = 1;
    for (u64 i = 0; i < page_count; i++) {
        page = MemoryManager::get()->alloc_page();
        if (page == 0)
            break;

        ret->pages.append(page);
    }

    if (page == 0) {
        delete ret;
        return nullptr;
    }

    return ret;
}
    
MemorySegment* MemorySegment::create_copy(MemorySegment* segment) {
    MemorySegment* ret = MemorySegment::create(segment->get_page_count(), segment->perms, segment->stack, segment->shared);
    copy(ret, segment);
    return ret;
}

MemorySegment::~MemorySegment() {
    for (auto page : pages)
        MemoryManager::get()->dealloc_page(page);
}

bool MemorySegment::copy_on_write() const {
    return (psegments.size() > 1) && (!shared) && (perms & VMEM_PAGE_WRITE);
}

VMemPerms MemorySegment::get_perms_for_proc(ProcessSegment* seg) const {
    VMemPerms ret = perms;
    if (copy_on_write())
        ret &= ~VMEM_PAGE_WRITE;
    return ret;
}

void MemorySegment::process_map(ProcessSegment* seg) {
    if (psegments.size() >= 1) {
        seg->set_segment(MemorySegment::create_copy(this));
        return;
    }

    bool prev_cow = copy_on_write();

    psegments.append(seg);

    seg->update_mapping();
    
    if (copy_on_write() != prev_cow) {
        for (auto& pseg : psegments)
            pseg->update_mapping();
    }
}

void MemorySegment::process_unmap(ProcessSegment* seg) {
    bool prev_cow = copy_on_write();

    usize idx = psegments.index_of(seg);
    if (idx == VEC_NOT_FOUND)
        return;

    psegments.remove(idx);

    if (copy_on_write() != prev_cow) {
        for (auto& pseg : psegments)
            pseg->update_mapping();
    }
}

void MemorySegment::set_page_count(usize count) {
    usize prev_count = get_page_count();

    if (count == prev_count)
        return;

    if (count < prev_count) {
        for (usize i = prev_count - 1; i >= count; i--) {
            MemoryManager::get()->dealloc_page(pages[i]);
            pages.remove(i);
        }
    } else {
        for (usize i = 0; i < count - prev_count; i++) {
            pages.append(MemoryManager::get()->alloc_page());
        }
    }
}

AccessFaultAction MemorySegment::access_fault(ProcessSegment* instigator, AccessFault fault) {
    if (fault.type != AccessFault::READ_ONLY)
        return AccessFaultAction::SEGFAULT;

    if (!copy_on_write())
        return AccessFaultAction::SEGFAULT;

    // TODO: Do a copy!
    panic("Copy on write not yet implemented!");

    instigator->set_segment(MemorySegment::create_copy(this));
    return AccessFaultAction::CONTINUE;
}

bool MemorySegment::copy(MemorySegment* dst, MemorySegment* src) {
    VirtualMemory* tempmem = VirtualMemoryManager::get()->create();

    u64 page_count = min(dst->get_page_count(), src->get_page_count());

    ROVec<u64> src_pages = src->get_pages();
    ROVec<u64> dst_pages = dst->get_pages();

    void* src_data = (void*)0x100000;
    void* dst_data = (void*)(0x100000 + page_count * ARCH_PAGE_SIZE);

    for (u64 i = 0; i < page_count; i++)
        tempmem->map_page(0x100 + i, src_pages[i], VMEM_PAGE_READ);

    for (u64 i = 0; i < page_count; i++)
        tempmem->map_page(0x100 + page_count + i, dst_pages[i], VMEM_PAGE_WRITE);

    VirtualMemory* prev = VirtualMemoryManager::get()->get_current();

    VirtualMemoryManager::get()->use(tempmem);

    memcpy(dst_data, src_data, page_count * ARCH_PAGE_SIZE);

    VirtualMemoryManager::get()->use(prev);

    VirtualMemoryManager::get()->destroy(tempmem);

    return true;
}

ProcessSegment::ProcessSegment(ProcessMemory* memory, MemorySegment* segment, PageRange range)
    : parent(memory), msegment(nullptr), prange(range)
{
    set_segment(segment);
}

ProcessSegment::~ProcessSegment() {
    if (msegment != nullptr) {
        msegment->process_unmap(this);
        if (msegment->should_delete())
            delete msegment;
    }
}

void ProcessSegment::update_mapping() {
    VirtualMemory* vmem = parent->get_vmem();

    VirtualMemoryManager::get()->use(vmem);

    VMemPerms perms  = msegment->get_perms_for_proc(this);
    ROVec<u64> pages = msegment->get_pages();

    for (u64 page = 0; page < prange.page_count(); page++) {
        if (page < msegment->get_page_count())
            vmem->map_page(prange.base + page, pages[page], perms);
        else
            vmem->unmap_page(prange.base + page);
    }
}

void ProcessSegment::resize(usize new_size) {
    usize old_size = prange.page_count();

    VirtualMemory* vmem = parent->get_vmem();
    
    VMemPerms perms  = msegment->get_perms_for_proc(this);
    ROVec<u64> pages = msegment->get_pages();

    if (new_size < old_size) {
        for (usize i = new_size; i >= old_size; i--) {
            vmem->unmap_page(prange.base + i);
        }
    } else {
        for (usize i = old_size; i < new_size; i++) {
            if (i < msegment->get_page_count())
                vmem->map_page(prange.base + i, pages[i], perms);
            else
                vmem->unmap_page(prange.base + i);
        }
    }
}

void ProcessSegment::set_segment(MemorySegment* segment) {
    if (msegment != nullptr) {
        msegment->process_unmap(this);
        if (msegment->should_delete())
            delete msegment;
    }

    msegment = segment;
    msegment->process_map(this);
}

ProcessMemory::ProcessMemory() {
    vmem = VirtualMemoryManager::get()->create();
}

ProcessMemory::~ProcessMemory() {
    for (auto seg : segments)
        delete seg;

    VirtualMemoryManager::get()->destroy(vmem);
}

ProcessSegment* ProcessMemory::map_segment(MemorySegment* segment, PageRange range) {
    for (auto seg : segments) {
        if (seg->get_pagerange().overlaps(range))
            return nullptr;
    }

    ProcessSegment* pseg = new ProcessSegment(this, segment, range);
    segments.append(pseg);

    return pseg;
}

void ProcessMemory::unmap_segment(ProcessSegment* segment) {
    usize idx = segments.index_of(segment);
    if (idx == VEC_NOT_FOUND)
        return;

    for (u64 i = segment->get_pagerange().base; i < segment->get_pagerange().limit; i++)
        vmem->unmap_page(i);

    segments.remove(idx);
    delete segment;
}

void ProcessMemory::unmap_all() {
    while (segments.size() > 0)
        unmap_segment(segments[0]);
}

ProcessSegment* ProcessMemory::get_segment_at(u64 base_page) {
    for (auto seg : segments) {
        if (seg->get_pagerange().base == base_page)
            return seg;
    }

    return nullptr;
}

ProcessMemory* ProcessMemory::fork() {
    ProcessMemory* dst = new ProcessMemory();

    dst->vmem = VirtualMemoryManager::get()->create();

    for (auto pseg : segments)
        dst->map_segment(pseg->get_mem_segment(), pseg->get_pagerange());

    return dst;
}

AccessFaultAction ProcessMemory::access_fault(AccessFault fault) {
    for (auto pseg : segments)
        if (pseg->get_pagerange().ptr_in_range(fault.address)) {
            return pseg->get_mem_segment()->access_fault(pseg, fault);
        }

    return AccessFaultAction::SEGFAULT;
}