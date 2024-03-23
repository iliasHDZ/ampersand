#pragma once

#include "memory.hpp"
#include <fd/fd.hpp>

struct ExecutableInfo {
    bool success;
    void* entry;
};

namespace Elf {

ExecutableInfo load_executable(FileDescription* fd, ProcessMemory* target);

};