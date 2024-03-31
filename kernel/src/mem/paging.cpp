#include "paging.hpp"
#include "memory.hpp"
#include "manager.hpp"
#include <proc/manager.hpp>

static VirtualMemoryManager vmm_instance;

VirtualMemory::~VirtualMemory() {}

u64 VirtualMemory::vir_addr_to_phy(u64 addr) {
    u64 idx = addr / ARCH_PAGE_SIZE;
    u64 off = addr % ARCH_PAGE_SIZE;

    u64 pidx = vir_paddr_to_phy(idx);

    return pidx * ARCH_PAGE_SIZE + off;
}

bool VirtualMemory::alloc_page(u64 vir_paddr, VMemPerms perms) {
    u64 pidx = MemoryManager::get()->alloc_page();

    if (!map_page(vir_paddr, pidx, perms)) {
        MemoryManager::get()->dealloc_page(pidx);
        return false;
    }

    return true;
}

void VirtualMemory::dealloc_page(u64 vir_paddr) {
    u64 pidx = vir_paddr_to_phy(vir_paddr);
    if (pidx == 0)
        return;
    
    MemoryManager::get()->dealloc_page(pidx);
    unmap_page(vir_paddr);
}

VirtualMemory* VirtualMemoryManager::create() {
    VirtualMemory* vmem = VirtualMemory::create_raw();

    vmems.append(vmem);

    return vmem;
}

void VirtualMemoryManager::destroy(VirtualMemory* vmem) {
    vmems.remove(vmem);

    VirtualMemory::destroy_raw(vmem);
}

DLChain<VirtualMemory>& VirtualMemoryManager::list_all() {
    return vmems;
}

void VirtualMemoryManager::init_manager() {
    VirtualMemory* kmem = VirtualMemory::get_kernel_memory();

    PageRange prange = PhysicalMemoryMap::get_kernel_range().to_pagerange();
    PageRange vrange = VirtualMemoryManager::get_kernel_range().to_pagerange();

    u64 pidx = prange.base;
    u64 vidx = vrange.base;
    for (; pidx < prange.limit && vidx < vrange.limit; pidx++, vidx++) {
        kmem->map_page(vidx, pidx, VMEM_PAGE_READ | VMEM_PAGE_WRITE | VMEM_PAGE_EXEC);

        MemoryManager::get()->reserve_page(pidx);
    }

    use(kmem);
}

void VirtualMemoryManager::use(VirtualMemory* vmem) {
    current_vmem = vmem;
    vmem->use();
}

u8* VirtualMemoryManager::driver_map(u64 phy_addr, u64 size) {
    VirtualMemory* kmem = VirtualMemory::get_kernel_memory();

    MemRange  mrange = MemRange::addr_size(phy_addr, size);
    PageRange prange = mrange.to_pagerange();

    u64 vir_paddr = kmem->driver_map(prange.base, prange.page_count());

    return (u8*)((usize)vir_paddr * ARCH_PAGE_SIZE) + (phy_addr % ARCH_PAGE_SIZE);
}

bool VirtualMemoryManager::access_fault(AccessFault fault) {
    Process* process = ProcessManager::get()->get_process_with_vmem(current_vmem);
    if (process == nullptr)
        return false;
    
    ProcessManager::get()->access_fault(process, fault);
}

VirtualMemoryManager* VirtualMemoryManager::get() {
    return &vmm_instance;
}

extern "C" {

extern monostate kernel_begin_addr;
extern monostate kernel_end_addr;

}

MemRange VirtualMemoryManager::get_kernel_range() {
    return MemRange::base_limit((u64)(u32)&kernel_begin_addr, (u64)(u32)&kernel_end_addr);
}

void VirtualMemoryManager::init() {
    vmm_instance.init_manager();
}