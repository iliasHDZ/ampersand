#include "boot.hpp"

#include "multiboot2.hpp"

#include <common.h>

#define BOOTLOADER_NAME_MAX_SIZE 128

bool bootloader_name_present = false;
char bootloader_name[BOOTLOADER_NAME_MAX_SIZE + 1];

void BootLoader::set_name(const char* name) {
    u32 i = 0;
    for (; i < BOOTLOADER_NAME_MAX_SIZE && name[i]; i++) {
        bootloader_name[i] = name[i];
    }

    bootloader_name[BOOTLOADER_NAME_MAX_SIZE] = 0;
    bootloader_name_present = true;
}

const char* BootLoader::get_name() {
    if (bootloader_name_present)
        return bootloader_name;
    
    return "UNKOWN BOOTLOADER";
}

void BootLoader::init() {
    if (multiboot2_init()) return;

    panic("Couldn't not determine the bootloader the kernel booted with");
}