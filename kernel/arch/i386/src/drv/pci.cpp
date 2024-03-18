#include <arch/drv/pci.hpp>

#include "../io.h"

#define PCI_CONFIG_ADDRESS_PORT 0xCF8
#define PCI_CONFIG_DATA_PORT    0xCFC

void pci_config_address(u32 address) {
    outl(PCI_CONFIG_ADDRESS_PORT, address);
}

u32 pci_config_read() {
    return inl(PCI_CONFIG_DATA_PORT);
}

void pci_config_write(u32 data) {
    outl(PCI_CONFIG_DATA_PORT, data);
}