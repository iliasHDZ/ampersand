#pragma once

#include <mem/paging.hpp>
#include <data/vec.hpp>

class ProcessSegment;

enum class AccessFaultAction {
    CONTINUE,
    SEGFAULT
};

class MemorySegment {
private:
    MemorySegment(u64 page_count, VMemPerms perms, bool shared = false);

public:
    static MemorySegment* create(u64 page_count, VMemPerms perms, bool stack = false, bool shared = false);
    
    static MemorySegment* create_copy(MemorySegment* segment);

    ~MemorySegment();

    inline u64 get_page_count() const { return pages.size(); };

    inline ROVec<u64> get_pages() { return pages; };

    inline VMemPerms get_perms() const { return perms; };

    bool copy_on_write() const;

    VMemPerms get_perms_for_proc(ProcessSegment* seg) const;

    void process_map(ProcessSegment* seg);

    void process_unmap(ProcessSegment* seg);

    void set_page_count(usize count);

    AccessFaultAction access_fault(ProcessSegment* instigator, AccessFault fault);

    static bool copy(MemorySegment* dst, MemorySegment* src);

    inline bool should_delete() const { return psegments.size() == 0; };

private:
    bool stack;
    bool shared;
    VMemPerms perms;

    Vec<ProcessSegment*> psegments;
    Vec<u64> pages;
};

class ProcessMemory;

class ProcessSegment {
public:
    ProcessSegment(ProcessMemory* memory, MemorySegment* segment, PageRange range);

    ~ProcessSegment();

    inline ProcessMemory* get_memory() { return parent; };
    inline PageRange get_pagerange()   { return prange; };
    
    inline MemorySegment* get_mem_segment() { return msegment; };

    void update_mapping();

    void resize(usize count);

    void set_segment(MemorySegment* segment);

private:
    MemorySegment* msegment;
    ProcessMemory* parent;
    
    PageRange prange;
};

class ProcessMemory {
public:
    ProcessMemory();

    ~ProcessMemory();

    ProcessSegment* map_segment(MemorySegment* segment, PageRange range);

    void unmap_segment(ProcessSegment* segment);

    void unmap_all();

    ProcessSegment* get_segment_at(u64 base_page);

    ProcessMemory* fork();

    inline ROVec<ProcessSegment*> get_segments() { return segments; };

    AccessFaultAction access_fault(AccessFault fault);

    inline VirtualMemory* get_vmem() { return vmem; };

private:
    friend class ProcessSegment;

private:
    VirtualMemory* vmem;

    Vec<ProcessSegment*> segments;
};