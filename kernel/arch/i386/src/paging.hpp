#include <mem/paging.hpp>

#define ARCH_PAGETAB_PRESENT 0x00000001
#define ARCH_PAGETAB_WRITE   0x00000002
#define ARCH_PAGETAB_USER    0x00000004
#define ARCH_PAGETAB_ACCESS  0x00000020
#define ARCH_PAGETAB_DIRTY   0x00000040
#define ARCH_PAGETAB_PADDR   0xfffff000

#define ARCH_PAGEDIR_PRESENT 0x00000001
#define ARCH_PAGEDIR_WRITE   0x00000002
#define ARCH_PAGEDIR_USER    0x00000004
#define ARCH_PAGEDIR_WRTHROU 0x00000008
#define ARCH_PAGEDIR_CACHETB 0x00000010
#define ARCH_PAGEDIR_ACCESS  0x00000020
#define ARCH_PAGEDIR_4MBPAGE 0x00000080
#define ARCH_PAGEDIR_PADDR   0xfffff000

#define ARCH_PAGING_TABLE_ENTRIES 1024

#define TEMP_PAGING_TABLE_COUNT 8

#define PAGE_ALIGNED __attribute__((aligned(4096)))

struct PACKED_STRUCT arch_paging_table {
    u32 entries[ARCH_PAGING_TABLE_ENTRIES];
};

class I386VirtualMemory : public VirtualMemory {
public:
    I386VirtualMemory(bool is_kernel);
    virtual ~I386VirtualMemory();

    bool map_page(u64 vir_paddr, u64 phy_paddr, usize perms) override;

    void unmap_page(u64 vir_paddr) override;

    u64 vir_paddr_to_phy(u64 vir_paddr) override;

    void use() override;

    u8* map_phy_paddr(u64 phy_paddr);

private:
    bool retain_page_tab(u32 idx);
    
    void release_page_tab(u32 idx);

    void set_page_tab(u32 idx, u64 page_tab, bool kernel_page);

    u64 get_page_table(u32 idx);

    void clear_page_tab(u32 idx);

private:
    u16 page_dir_map_count[1024] = { 0 };

    bool is_kernel;

    u64 page_dir;
};