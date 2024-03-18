#include "pci.hpp"

#include <arch/drv/pci.hpp>

#include <logger.hpp>

#include "../ide.hpp"

#define PCI_DEVICE_ADDRESS(B, D, F, O) (((u32)B << 16) | ((u32)D << 11) | ((u32)F << 8) | ((u32)O & 0xFC) | ((u32)0x80000000))

PCIManager PCIManager::instance;

bool PCIManager::is_valid_device(u8 bus, u8 device, u8 func) {
    pci_config_address(PCI_DEVICE_ADDRESS(bus, device, func, 0));
    return (pci_config_read() & 0xffff) != 0xffff;
}

PCIDevice* PCIManager::add_device(u8 bus, u8 device, u8 func) {
    if (!is_valid_device(bus, device, func))
        return nullptr;

    PCIDevice* pci_device = new PCIDevice(bus, device, func);

    for (u32 i = 0; i < 16; i++) {
        pci_config_address(PCI_DEVICE_ADDRESS(bus, device, func, i * 4));
        pci_device->config[i] = pci_config_read();
    }

    devices.append(pci_device);

    PCIDeviceDriver* driver = fetch_driver_for_device(pci_device);
    if (driver)
        add_driver(driver);

    return pci_device;
}

void PCIManager::check_device(u8 bus, u8 device) {
    PCIDevice* dev = add_device(bus, device, 0);
    if (dev == nullptr)
        return;

    if (dev->header_type() & 0x80) {
        for (u8 func = 1; func < 8; func++)
            add_device(bus, device, func);
    }
}

void PCIManager::find_all_devices() {
    for (u16 bus = 0; bus < 256; bus++) {
        for (u8 device = 0; device < 32; device++) {
            check_device(bus, device);
        }
    }
}

void PCIManager::init_manager() {
    Log::INFO("PCIManager") << "Initializing devices...\n";

    find_all_devices();

    Log::INFO("PCIManager") << "PCI Devices: (bus, device, function) [class code, subclass, progif] vendor:device\n";

    for (auto& device : devices) {
        TerminalOutputStream& o = Log::INFO("PCIManager");
        o << Out::dec() << "  - (" << device.get_bus_num() << ", " << device.get_device_num() << ", " << device.get_func_num() << ") ";
        o << " [" << device.class_code() << ", " << device.subclass() << ", " << device.prog_if() << "] ";
        o << Out::phex(4) << device.vendor_id() << ":" << device.device_id() << '\n';
    }
}

PCIDeviceDriver* PCIManager::fetch_driver_for_device(PCIDevice* device) {
    if (device->class_code() == 1 && device->subclass() == 1)
        return new IDEController(device);
    
    return nullptr;
}

void PCIManager::add_driver(PCIDeviceDriver* driver) {
    if (driver == nullptr)
        return;

    if (!driver->init()) {
        delete driver;
        return;
    }

    drivers.append(driver);
}

DLChain<PCIDevice>& PCIManager::get_devices() {
    return devices;
}

void PCIManager::init() {
    instance.init_manager();
}

PCIManager* PCIManager::get() {
    return &instance;
}