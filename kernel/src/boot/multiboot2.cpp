#include "multiboot2.hpp"
#include "multiboot2.h"

#include "boot.hpp"

#include <mem/memory.hpp>

#include <logger.hpp>

struct multiboot_info {
    multiboot_uint32_t total_size;
    multiboot_uint32_t reserved;
};

extern "C" {
    
multiboot_info* multiboot_info_structure;
u32 multiboot_magic_number;

}

static PhysicalMemoryType get_mem_type_from_multiboot_type(usize type) {
    switch (type) {
    case MULTIBOOT_MEMORY_AVAILABLE:
        return PhysicalMemoryType::AVAILABLE;
    case MULTIBOOT_MEMORY_RESERVED:
        return PhysicalMemoryType::RESERVED;
    case MULTIBOOT_MEMORY_ACPI_RECLAIMABLE:
        return PhysicalMemoryType::ACPI_RECLAIMABLE;
    case MULTIBOOT_MEMORY_NVS:
        return PhysicalMemoryType::NVS;
    case MULTIBOOT_MEMORY_BADRAM:
        return PhysicalMemoryType::BADRAM;
    }

    return PhysicalMemoryType::BADRAM;
}

void multiboot2_handle_memory_tag(multiboot_tag_mmap* tag) {
    u8* ptr = (u8*)(tag->entries);
    u8* end = (u8*)tag + tag->size;

    for (; ptr < end; ptr += tag->entry_size) {
        multiboot_mmap_entry* entry = (multiboot_mmap_entry*)ptr;

        PhysicalMemoryMap::add_range(MemRange::addr_size(entry->addr, entry->len), get_mem_type_from_multiboot_type(entry->type));
    }
}

void multiboot2_handle_bootloader_name(multiboot_tag_string* tag) {
    BootLoader::set_name(tag->string);
}

bool multiboot2_init() {
    if (multiboot_magic_number != MULTIBOOT2_BOOTLOADER_MAGIC)
        return false;

    u8* ptr = (u8*)multiboot_info_structure + sizeof(multiboot_info);
    u8* end = ptr + (multiboot_info_structure->total_size - sizeof(multiboot_info));

    while (ptr < end) {
        multiboot_tag* tag = (multiboot_tag*)ptr;

        switch (tag->type) {
        case MULTIBOOT_TAG_TYPE_BOOT_LOADER_NAME:
            multiboot2_handle_bootloader_name((multiboot_tag_string*)tag);
            break;
        case MULTIBOOT_TAG_TYPE_MMAP:
            multiboot2_handle_memory_tag((multiboot_tag_mmap*)tag);
            break;
        case MULTIBOOT_TAG_TYPE_END:
            return true;
        }

        u8* next_ptr = (u8*)(((usize)(ptr + tag->size) / MULTIBOOT_TAG_ALIGN) * MULTIBOOT_TAG_ALIGN);
        if (next_ptr != ptr)
            next_ptr += MULTIBOOT_TAG_ALIGN;

        ptr = next_ptr;
    }

    return true;
}