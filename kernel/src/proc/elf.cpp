#include "elf.hpp"

#include <logger.hpp>

typedef u32 Elf32_Addr;
typedef u16 Elf32_Half;
typedef u32 Elf32_Off;
typedef i32 Elf32_Sword;
typedef u32 Elf32_Word;

#define EI_NIDENT 16

struct Elf32_Ehdr {
    unsigned char e_ident[EI_NIDENT];
    Elf32_Half e_type;
    Elf32_Half e_machine;
    Elf32_Word e_version;
    Elf32_Addr e_entry;
    Elf32_Off e_phoff;
    Elf32_Off e_shoff;
    Elf32_Word e_flags;
    Elf32_Half e_ehsize;
    Elf32_Half e_phentsize;
    Elf32_Half e_phnum;
    Elf32_Half e_shentsize;
    Elf32_Half e_shnum;
    Elf32_Half e_shstrndx;
};

struct Elf32_Phdr {
    Elf32_Word p_type;
    Elf32_Off p_offset;
    Elf32_Addr p_vaddr;
    Elf32_Addr p_paddr;
    Elf32_Word p_filesz;
    Elf32_Word p_memsz;
    Elf32_Word p_flags;
    Elf32_Word p_align;
};

#define ET_NONE   0

#define ET_REL    1
#define ET_EXEC   2
#define ET_DYN    3
#define ET_CORE   4
#define ET_LOPROC 0xff00
#define ET_HIPROC 0xffff

#define EM_M32 1
#define EM_SPARC 2
#define EM_386 3
#define EM_68K 4
#define EM_88K 5
#define EM_860 7
#define EM_MIPS 8
#define EM_MIPS_RS4_BE 10

#ifdef ARCH_I386
    #define EM_CURRENT EM_386
    #define EI_CURRENT_CLASS 1
#else
    #define EM_CURRENT ET_NONE
    #define EI_CURRENT_CLASS 0
#endif

#define EI_MAG0    0
#define EI_MAG1    1
#define EI_MAG2    2
#define EI_MAG3    3
#define EI_CLASS   4
#define EI_DATA    5
#define EI_VERSION 6
#define EI_PAD     7

#define PT_NULL    0
#define PT_LOAD    1
#define PT_DYNAMIC 2
#define PT_INTERP  3
#define PT_NOTE    4
#define PT_SHLIB   5
#define PT_PHDR    6
#define PT_LOPROC  0x70000000
#define PT_HIPROC  0x7fffffff

#define PF_X        0x1
#define PF_W        0x2
#define PF_R        0x4
#define PF_MASKPROC 0xf0000000

static char elf_magic[] = { 0x7F, 'E', 'L', 'F' };

namespace Elf {

bool validate_header(Elf32_Ehdr* header) {
    if (!memeq(header->e_ident, elf_magic, 4)) {
        Log::ERR("Elf") << "Failed loading: Invalid magic\n";
        return false;
    }
    
    if (header->e_machine != EM_CURRENT) {
        Log::ERR("Elf") << "Failed loading: Incorrect target machine\n";
        return false;
    }
    
    if (header->e_ident[EI_CLASS] != EI_CURRENT_CLASS) {
        Log::ERR("Elf") << "Failed loading: Invalid machine class\n";
        return false;
    }

    return true;
}

bool load_program_header_entry(Elf32_Phdr* phe, FileDescription* fd, ProcessMemory* target) {
    if (phe->p_type != PT_LOAD && phe->p_type != PT_DYNAMIC)
        return true;

    if (phe->p_type == PT_DYNAMIC) {
        Log::ERR("Elf") << "Failed loading: Dynamic linking not yet supported\n";
        return false;
    }

    if (phe->p_vaddr == 0) {
        Log::ERR("Elf") << "Failed loading: Program header entry has no virtual address\n";
        return false;
    }

    MemRange  mrange = MemRange::addr_size(phe->p_vaddr, phe->p_memsz);
    PageRange prange = mrange.to_pagerange();

    VMemPerms perms = 0;

    if (phe->p_flags & PF_R) perms |= VMEM_PAGE_READ;
    if (phe->p_flags & PF_W) perms |= VMEM_PAGE_WRITE;
    if (phe->p_flags & PF_X) perms |= VMEM_PAGE_EXEC;

    MemorySegment* mseg = MemorySegment::create(prange.page_count(), perms);

    if (mseg == nullptr) {
        Log::ERR("Elf") << "Failed loading: Could not allocate memory\n";
        return false;
    }

    ProcessSegment* pseg = target->map_segment(mseg, prange);

    if (pseg == nullptr) {
        Log::ERR("Elf") << "Failed loading: Could not create process segment\n";
        delete mseg;
        return false;
    }

    VirtualMemory* prev = VirtualMemoryManager::get()->get_current();

    VirtualMemoryManager::get()->use(target->get_vmem());

    usize copy_size = min(phe->p_memsz, phe->p_filesz);

    if (copy_size > 0) {
        if (fd->read((void*)mrange.base, phe->p_offset, copy_size) != copy_size) {
            Log::ERR("Elf") << "Failed loading: Could not read program segment\n";
            target->unmap_segment(pseg);
            return false;
        }
    }

    if (phe->p_memsz > phe->p_filesz) {
        memset((void*)(mrange.base + copy_size), 0, phe->p_memsz - phe->p_filesz);
    }
    
    VirtualMemoryManager::get()->use(prev);
    return true;
}

bool load_program(Elf32_Ehdr* header, FileDescription* fd, ProcessMemory* target) {
    if (header->e_phoff == 0) {
        Log::ERR("Elf") << "Failed loading: No program header table\n";
        return false;
    }

    usize ph_table_size = header->e_phentsize * header->e_phnum;

    u8* ph_table = new u8[ph_table_size];

    if (fd->read(ph_table, header->e_phoff, ph_table_size) != ph_table_size) {
        Log::ERR("Elf") << "Failed loading: Could not read program header table\n";
        delete[] ph_table;
        return false;
    }
    
    for (u8* phe = ph_table; phe < ph_table + ph_table_size; phe += header->e_phentsize) {
        if (!load_program_header_entry((Elf32_Phdr*)phe, fd, target)) {
            delete[] ph_table;
            return false;
        }
    }
    
    delete[] ph_table;
    return true;
}

ExecutableInfo load_executable(FileDescription* fd, ProcessMemory* target) {
    Elf32_Ehdr header;

    usize ret = fd->read(&header, 0, sizeof(Elf32_Ehdr));
    if (ret != sizeof(Elf32_Ehdr)) {
        Log::ERR("Elf") << "Failed loading: Could not read header\n";
        return { .success = false };
    }
    
    if (!validate_header(&header))
        return { .success = false };
    
    if (header.e_type != ET_EXEC) {
        Log::ERR("Elf") << "Failed loading: Not an executable\n";
        return { .success = false };
    }
    
    if (!load_program(&header, fd, target))
        return { .success = false };

    return {
        .success = true,
        .entry = (void*)(header.e_entry)
    };
}

};