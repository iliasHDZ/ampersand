#include "paging.hpp"

static arch_paging_table PAGE_ALIGNED arch_temp_paging_tables[TEMP_PAGING_TABLE_COUNT];
static bool arch_temp_paging_tables_alloc[TEMP_PAGING_TABLE_COUNT];

static arch_paging_table PAGE_ALIGNED kernel_page_dir = { 0 };
static u16 kernel_page_dir_map_count[1024] = { 0 };

static I386VirtualMemory kernel_vmem(true);

static arch_paging_table* arch_alloc_table() {
    if (is_alloc_available()) {
        auto table = (arch_paging_table*)kmalloc_align(sizeof(arch_paging_table), 12);

        memset(table, 0, sizeof(arch_paging_table));

        if (table != nullptr)
            return table;
    }

    for (usize i = 0; i < TEMP_PAGING_TABLE_COUNT; i++)
        if (!arch_temp_paging_tables_alloc[i]) {
            arch_temp_paging_tables_alloc[i] = true;

            auto table = &arch_temp_paging_tables[i];

            memset(table, 0, sizeof(arch_paging_table));
            return table;
        }
    
    panic("Ran out of temporary paging tables");
    return nullptr;
}

static void arch_free_table(arch_paging_table* table) {
    if (table >= arch_temp_paging_tables && table < (arch_temp_paging_tables + TEMP_PAGING_TABLE_COUNT)) {
        usize idx = (usize)(table - arch_temp_paging_tables) >> 2;

        arch_temp_paging_tables_alloc[idx] = false;
        return;
    }

    delete table;
}

I386VirtualMemory::I386VirtualMemory(bool is_kernel)
    : is_kernel(is_kernel)
{
    if (is_kernel) {
        page_dir = &kernel_page_dir;
        return;
    }

    page_dir = arch_alloc_table();

    memcpy(page_dir, &kernel_page_dir, sizeof(arch_paging_table));
}

I386VirtualMemory::~I386VirtualMemory() {
    if (!is_kernel)
        arch_free_table(page_dir);
}

bool I386VirtualMemory::map_page(u64 vir_paddr, u64 phy_paddr, usize perms) {
    u16 dir_idx = vir_paddr >> 10;
    u16 tab_idx = vir_paddr & 0x3ff;

    retain_page_tab(dir_idx);

    arch_paging_table* table = get_page_table(dir_idx);

    if (table->entries[tab_idx] & ARCH_PAGETAB_PRESENT) {
        release_page_tab(dir_idx);
        return false;
    }

    u32 tab_ent = (phy_paddr << 12) | ARCH_PAGETAB_PRESENT;

    if ((perms & VMEM_PAGE_WRITE) || (perms & VMEM_PAGE_EXEC))
        tab_ent |= ARCH_PAGETAB_WRITE;

    table->entries[tab_idx] = tab_ent;
    return true;
}

void I386VirtualMemory::unmap_page(u64 vir_paddr) {
    u16 dir_idx = vir_paddr >> 10;
    u16 tab_idx = vir_paddr & 0x3ff;

    arch_paging_table* table = get_page_table(dir_idx);
    
    if (table == nullptr)
        return;

    if ((table->entries[tab_idx] & ARCH_PAGETAB_PRESENT) == 0)
        return;

    table->entries[tab_idx] = 0;
    release_page_tab(dir_idx);
}

u64 I386VirtualMemory::vir_paddr_to_phy(u64 vir_paddr) {
    u16 dir_idx = vir_paddr >> 10;
    u16 tab_idx = vir_paddr & 0x3ff;

    arch_paging_table* table = get_page_table(dir_idx);
    if (table == nullptr)
        return 0;

    if ((table->entries[tab_idx] & ARCH_PAGETAB_PRESENT) == 0)
        return 0;

    return table->entries[tab_idx] >> 12;
}

bool I386VirtualMemory::retain_page_tab(u32 idx) {
    u32 dir_ent = page_dir->entries[idx];

    page_dir_map_count[idx]++;

    if (dir_ent & ARCH_PAGEDIR_PRESENT)
        return true;

    page_dir_map_count[idx] = 1;

    arch_paging_table* page_tab = arch_alloc_table();
    if (page_tab == nullptr)
        return false;

    set_page_tab(idx, page_tab, is_kernel);
    return true;
}

void I386VirtualMemory::release_page_tab(u32 idx) {
    page_dir_map_count[idx]--;

    if (page_dir_map_count[idx] > 0)
        return;
    
    arch_free_table(get_page_table(idx));
    clear_page_tab(idx);
}

void I386VirtualMemory::set_page_tab(u32 idx, arch_paging_table* page_tab, bool kernel_page) {
    u32 dir_ent = (u32)page_tab | ARCH_PAGEDIR_PRESENT | ARCH_PAGEDIR_WRITE | ARCH_PAGEDIR_USER;

    if (!kernel_page)
        dir_ent |= ARCH_PAGEDIR_USER;

    if (is_kernel) {
        for (auto& vmem : VirtualMemoryManager::get()->list_all()) {
            I386VirtualMemory* avmem = (I386VirtualMemory*)&vmem;
            avmem->set_page_tab(idx, page_tab, true);
        }
    }
    
    page_dir->entries[idx] = dir_ent;
}

arch_paging_table* I386VirtualMemory::get_page_table(u32 idx) {
    return (arch_paging_table*)(page_dir->entries[idx] & ARCH_PAGEDIR_PADDR);
}

void I386VirtualMemory::clear_page_tab(u32 idx) {
    page_dir->entries[idx] = 0;

    if (is_kernel) {
        for (auto& vmem : VirtualMemoryManager::get()->list_all()) {
            I386VirtualMemory* avmem = (I386VirtualMemory*)&vmem;
            avmem->clear_page_tab(idx);
        }
    }
}

VirtualMemory* VirtualMemory::get_kernel_memory() {
    return &kernel_vmem;
}

VirtualMemory* VirtualMemory::create_raw() {
    return new I386VirtualMemory(false);
}

void VirtualMemory::destroy_raw(VirtualMemory* vmem) {
    delete vmem;
}