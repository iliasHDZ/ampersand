#pragma once

#include <common.h>

void pci_config_address(u32 address);

u32 pci_config_read();

void pci_config_write(u32 data);