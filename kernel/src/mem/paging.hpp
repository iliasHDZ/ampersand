#pragma once

#include <common.h>
#include <data/chain.hpp>

#include "memory.hpp"

#define VMEM_PAGE_READ   0b00000001
#define VMEM_PAGE_WRITE  0b00000010
#define VMEM_PAGE_EXEC   0b00000100

class VirtualMemoryManager;

class VirtualMemory : public DLChainItem {
public:
    virtual ~VirtualMemory();

    virtual bool map_page(u64 vir_paddr, u64 phy_paddr, usize perms) = 0;

    virtual void unmap_page(u64 vir_paddr) = 0;

    virtual u64 vir_paddr_to_phy(u64 vir_paddr) = 0;

    virtual u64 driver_map(u64 phy_paddr, u64 page_count) = 0;

    u64 vir_addr_to_phy(u64 addr);

    bool alloc_page(u64 vir_paddr, usize perms);

    void dealloc_page(u64 vir_paddr);

    static VirtualMemory* get_kernel_memory();

private:
    virtual void use() = 0;

    static VirtualMemory* create_raw();

    static void destroy_raw(VirtualMemory* vmem);

    friend class VirtualMemoryManager;
};

class VirtualMemoryManager {
public:
    VirtualMemory* create();

    void destroy(VirtualMemory* vmem);

    DLChain<VirtualMemory>& list_all();

    void init_manager();

    void use(VirtualMemory* vmem);

    inline VirtualMemory* get_current() { return current_vmem; };

    u8* driver_map(u64 phy_addr, u64 size);

public:
    static VirtualMemoryManager* get();

    static MemRange get_kernel_range();
    
    static void init();

private:
    DLChain<VirtualMemory> vmems;
    VirtualMemory* current_vmem = nullptr;
};