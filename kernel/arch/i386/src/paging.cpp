#include "paging.hpp"
#include <logger.hpp>
#include <mem/manager.hpp>

static arch_paging_table PAGE_ALIGNED arch_temp_paging_tables[TEMP_PAGING_TABLE_COUNT];
static bool arch_temp_paging_tables_alloc[TEMP_PAGING_TABLE_COUNT];

static arch_paging_table PAGE_ALIGNED kernel_page_dir  = { 0 };
static arch_paging_table PAGE_ALIGNED kernel_last_page = { 0 };
static u16 kernel_page_dir_map_count[1024] = { 0 };

static I386VirtualMemory kernel_vmem(true);

static u16 kernel_temp_page_counter;

static bool paging_initiated = false;

static u64 arch_vir_paddr_to_phy(u64 vir_paddr) {
    auto prange = PhysicalMemoryMap::get_kernel_range().to_pagerange();
    auto vrange = VirtualMemoryManager::get_kernel_range().to_pagerange();

    if (vrange.page_in_range(vir_paddr))
        return vir_paddr - vrange.base + prange.base;

    auto vmem = VirtualMemoryManager::get()->get_current();
    if (vmem == nullptr)
        return 0;

    return vmem->vir_paddr_to_phy(vir_paddr);
}

static u8* arch_map_phy_paddr(u64 phy_paddr) {
    auto prange = PhysicalMemoryMap::get_kernel_range().to_pagerange();
    auto vrange = VirtualMemoryManager::get_kernel_range().to_pagerange();

    if (prange.page_in_range(phy_paddr))
        return (u8*)((phy_paddr - prange.base + vrange.base) * ARCH_PAGE_SIZE);

    if (!paging_initiated)
        return 0;

    auto vmem = VirtualMemory::get_kernel_memory();
    if (vmem == nullptr)
        return 0;

    return ((I386VirtualMemory*)vmem)->map_phy_paddr(phy_paddr);
}

static u64 arch_alloc_table() {
    if (!paging_initiated) {
        for (usize i = 0; i < TEMP_PAGING_TABLE_COUNT; i++)
            if (!arch_temp_paging_tables_alloc[i]) {
                arch_temp_paging_tables_alloc[i] = true;

                auto table = &arch_temp_paging_tables[i];

                memset(table, 0, sizeof(arch_paging_table));
                return arch_vir_paddr_to_phy((u64)(u32)table / ARCH_PAGE_SIZE);
            }
    }

    u64 page = MemoryManager::get()->alloc_page();
    if (page) {
        memset(arch_map_phy_paddr(page), 0, sizeof(arch_paging_table));

        return page;
    }
    
    panic("Ran out of paging tables");
    return 0;
}

static void arch_free_table(u64 table_paddr) {
    arch_paging_table* table = (arch_paging_table*)arch_map_phy_paddr(table_paddr);
    
    if (table >= arch_temp_paging_tables && table < (arch_temp_paging_tables + TEMP_PAGING_TABLE_COUNT)) {
        usize idx = (usize)(table - arch_temp_paging_tables) >> 2;

        arch_temp_paging_tables_alloc[idx] = false;
        return;
    }
    
    MemoryManager::get()->dealloc_page(table_paddr);
}

I386VirtualMemory::I386VirtualMemory(bool is_kernel)
    : is_kernel(is_kernel)
{
    if (is_kernel) {
        page_dir = arch_vir_paddr_to_phy(((u64)(u32)&kernel_page_dir) / ARCH_PAGE_SIZE);

        set_page_tab(1023, arch_vir_paddr_to_phy(((u64)(u32)&kernel_last_page) / ARCH_PAGE_SIZE), is_kernel);
        page_dir_map_count[1023] = 0xffff;
        return;
    }

    page_dir = arch_alloc_table();

    memcpy(arch_map_phy_paddr(page_dir), &kernel_page_dir, sizeof(arch_paging_table));
}

I386VirtualMemory::~I386VirtualMemory() {
    if (!is_kernel)
        arch_free_table(page_dir);
}

bool I386VirtualMemory::map_page(u64 vir_paddr, u64 phy_paddr, usize perms) {
    u16 dir_idx = vir_paddr >> 10;
    u16 tab_idx = vir_paddr & 0x3ff;

    retain_page_tab(dir_idx);

    u64 table_paddr = get_page_table(dir_idx);
    if (table_paddr == 0)
        return false;
    
    arch_paging_table* table = (arch_paging_table*)arch_map_phy_paddr(table_paddr);

    if (table->entries[tab_idx] & ARCH_PAGETAB_PRESENT)
        release_page_tab(dir_idx);

    u32 tab_ent = (phy_paddr << 12) | ARCH_PAGETAB_PRESENT;

    if (perms & VMEM_PAGE_WRITE)
        tab_ent |= ARCH_PAGETAB_WRITE;

    table->entries[tab_idx] = tab_ent;
    return true;
}

void I386VirtualMemory::unmap_page(u64 vir_paddr) {
    u16 dir_idx = vir_paddr >> 10;
    u16 tab_idx = vir_paddr & 0x3ff;

    u64 table_paddr = get_page_table(dir_idx);
    if (table_paddr == 0)
        return;
    
    arch_paging_table* table = (arch_paging_table*)arch_map_phy_paddr(table_paddr);

    if ((table->entries[tab_idx] & ARCH_PAGETAB_PRESENT) == 0)
        return;

    table->entries[tab_idx] = 0;
    release_page_tab(dir_idx);
}

u64 I386VirtualMemory::vir_paddr_to_phy(u64 vir_paddr) {
    u16 dir_idx = vir_paddr >> 10;
    u16 tab_idx = vir_paddr & 0x3ff;

    u64 table_paddr = get_page_table(dir_idx);
    if (table_paddr == 0)
        return 0;

    arch_paging_table* table = (arch_paging_table*)arch_map_phy_paddr(table_paddr);

    if ((table->entries[tab_idx] & ARCH_PAGETAB_PRESENT) == 0)
        return 0;

    return table->entries[tab_idx] >> 12;
}

u64 I386VirtualMemory::driver_map(u64 phy_paddr, u64 page_count) {
    driver_map_top -= page_count;

    for (u32 i = 0; i < page_count; i++)
        map_page(driver_map_top + i, phy_paddr + i, VMEM_PAGE_READ | VMEM_PAGE_WRITE);
    
    return driver_map_top;
}

void I386VirtualMemory::use() {
    asm volatile("mov %0, %%cr3":: "r"(page_dir * ARCH_PAGE_SIZE));
    paging_initiated = true;
}

u8* I386VirtualMemory::map_phy_paddr(u64 phy_paddr) {
    if (!paging_initiated)
        return nullptr;

    u32 paddr = 0xffc00 + kernel_temp_page_counter;
    kernel_temp_page_counter = (kernel_temp_page_counter + 1) & 0x3ff;

    map_page(paddr, phy_paddr, VMEM_PAGE_READ | VMEM_PAGE_WRITE);
    
    return (u8*)(paddr * ARCH_PAGE_SIZE);
}

bool I386VirtualMemory::retain_page_tab(u32 idx) {
    if (page_dir_map_count[idx] == 0xffff)
        return true;
    
    arch_paging_table* page_dir_vir = (arch_paging_table*)arch_map_phy_paddr(page_dir);

    u32 dir_ent = page_dir_vir->entries[idx];

    page_dir_map_count[idx]++;

    if (dir_ent & ARCH_PAGEDIR_PRESENT)
        return true;

    page_dir_map_count[idx] = 1;

    u64 page_tab = arch_alloc_table();
    if (page_tab == 0)
        return false;

    set_page_tab(idx, page_tab, is_kernel);
    return true;
}

void I386VirtualMemory::release_page_tab(u32 idx) {
    if (page_dir_map_count[idx] == 0xffff)
        return;

    page_dir_map_count[idx]--;

    if (page_dir_map_count[idx] > 0)
        return;
    
    arch_free_table(get_page_table(idx));
    clear_page_tab(idx);
}

void I386VirtualMemory::set_page_tab(u32 idx, u64 page_tab, bool kernel_page) {
    u32 dir_ent = ((u32)page_tab << 12) | ARCH_PAGEDIR_PRESENT | ARCH_PAGEDIR_WRITE;

    if (!kernel_page)
        dir_ent |= ARCH_PAGEDIR_USER;

    if (is_kernel) {
        for (auto& vmem : VirtualMemoryManager::get()->list_all()) {
            I386VirtualMemory* avmem = (I386VirtualMemory*)&vmem;
            avmem->set_page_tab(idx, page_tab, true);
        }
    }
    
    arch_paging_table* page_dir_vir = (arch_paging_table*)arch_map_phy_paddr(page_dir);
    page_dir_vir->entries[idx] = dir_ent;
}

u64 I386VirtualMemory::get_page_table(u32 idx) {
    arch_paging_table* page_dir_vir = (arch_paging_table*)arch_map_phy_paddr(page_dir);

    return (page_dir_vir->entries[idx] & ARCH_PAGEDIR_PADDR) >> 12;
}

void I386VirtualMemory::clear_page_tab(u32 idx) {
    arch_paging_table* page_dir_vir = (arch_paging_table*)arch_map_phy_paddr(page_dir);

    page_dir_vir->entries[idx] = 0;

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