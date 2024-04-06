#pragma once

#include <common.h>
#include <common/rovec.hpp>
#include <arch/arch.hpp>

#define MEMORY_MAP_MAX_RANGE_COUNT 32

struct PageRange;

struct MemRange {
public:
    u64 base;
    u64 limit;

    inline u64 size() const {
        return limit - base;
    }

    inline static MemRange addr_size(u64 addr, u64 size) {
        return MemRange { addr, addr + size };
    }

    inline static MemRange base_limit(u64 base, u64 limit) {
        return MemRange { base, limit };
    }

    inline bool is_addressable() const {
        usize lbase = base;
        return (u64)lbase == base;
    }

    inline PageRange to_pagerange() const;

    inline PageRange to_inner_pagerange() const;

    template <typename T>
    inline bool in_range(T ptr) const {
        return (u64)ptr >= base && (u64)ptr < limit;
    }
};

struct PageRange {
public:
    u64 base;
    u64 limit;

    inline u64 page_count() const {
        return limit - base;
    }

    inline u64 size() const {
        return page_count() * ARCH_PAGE_SIZE;
    }

    inline static PageRange base_count(u64 base, u64 count) {
        return PageRange { base, base + count };
    }

    inline static PageRange base_limit(u64 base, u64 limit) {
        return PageRange { base, limit };
    }

    inline bool overlaps(PageRange range) const {
        return limit > range.base && range.limit > base;
    }

    inline MemRange to_memrange() const {
        return MemRange::base_limit(base * ARCH_PAGE_SIZE, limit * ARCH_PAGE_SIZE);
    }

    template <typename T>
    inline bool page_in_range(T ptr) const {
        return (u64)ptr >= base && (u64)ptr < limit;
    }

    template <typename T>
    inline bool ptr_in_range(T ptr) const {
        return page_in_range(ptr / ARCH_PAGE_SIZE);
    }
};

inline PageRange MemRange::to_pagerange() const {
    u64 plimit = limit / ARCH_PAGE_SIZE;
    if (limit % ARCH_PAGE_SIZE)
        plimit++;

    return PageRange::base_limit(base / ARCH_PAGE_SIZE, plimit);
}

inline PageRange MemRange::to_inner_pagerange() const {
    u64 pbase = base / ARCH_PAGE_SIZE;
    if (base % ARCH_PAGE_SIZE)
        pbase++;

    return PageRange::base_limit(pbase, limit / ARCH_PAGE_SIZE);
}

enum class PhysicalMemoryType {
    AVAILABLE,
    RESERVED,
    ACPI_RECLAIMABLE,
    NVS,
    BADRAM
};

const char* PhysicalMemoryType_to_string(PhysicalMemoryType type);

struct PhysicalMemoryRange {
    MemRange range;
    PhysicalMemoryType type;
};

namespace PhysicalMemoryMap {
    bool add_range(MemRange range, PhysicalMemoryType type);

    ROVec<PhysicalMemoryRange> get_ranges();

    MemRange get_kernel_range();

    void log_memory_map();
};
