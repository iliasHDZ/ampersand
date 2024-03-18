#include "memory.hpp"

#include "logger.hpp"

static PhysicalMemoryRange memory_map[MEMORY_MAP_MAX_RANGE_COUNT];

static u32 memory_map_index = 0;

const char* PhysicalMemoryType_to_string(PhysicalMemoryType type) {
    switch (type) {
    case PhysicalMemoryType::AVAILABLE:
        return "AVAILABLE";
    case PhysicalMemoryType::RESERVED:
        return "RESERVED";
    case PhysicalMemoryType::ACPI_RECLAIMABLE:
        return "ACPI_RECLAIMABLE";
    case PhysicalMemoryType::NVS:
        return "NVS";
    case PhysicalMemoryType::BADRAM:
    default:
        return "BADRAM";
    }
}

namespace PhysicalMemoryMap {

bool add_range(MemRange range, PhysicalMemoryType type) {
    if (memory_map_index >= MEMORY_MAP_MAX_RANGE_COUNT)
        return false;

    memory_map[memory_map_index++] = { range, type };

    return true;
}

ROVec<PhysicalMemoryRange> get_ranges() {
    return ROVec(memory_map, memory_map_index);
}

extern "C" {

extern monostate kernel_begin_phys;
extern monostate kernel_end_phys;

}

MemRange get_kernel_range() {
    return MemRange::base_limit(0x0fffffff & (u64)&kernel_begin_phys, 0x0fffffff & (u64)&kernel_end_phys);
}

void log_memory_map() {
    Log::INFO() << "Physical Memory Map:\n";

    auto list = get_ranges();
    for (auto& entry : list) {
        auto stream = Log::INFO();
        stream << Out::phex(16) << "    " << entry.range.base << " - " << entry.range.limit << " (";
        stream.write_size(entry.range.size());
        stream << ") : " << PhysicalMemoryType_to_string(entry.type) << '\n';
    }
}

}