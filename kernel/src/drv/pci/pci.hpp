#pragma once

#include <common.h>
#include <common/chain.hpp>
#include "pci_device.hpp"

class PCIDeviceDriver : public DLChainItem {
public:
    inline PCIDeviceDriver(PCIDevice* pci_device)
        : pci_device(pci_device) {}

    virtual bool init() = 0;

protected:
    PCIDevice* pci_device;

};

class PCIManager {
private:
    inline PCIManager() {};

    bool is_valid_device(u8 bus, u8 device, u8 func);

    PCIDevice* add_device(u8 bus, u8 device, u8 func);

    void check_device(u8 bus, u8 device);

    void find_all_devices();

    void init_manager();

    PCIDeviceDriver* fetch_driver_for_device(PCIDevice* device);

    void add_driver(PCIDeviceDriver* driver);

public:
    DLChain<PCIDevice>& get_devices();

public:
    static void init();

    static PCIManager* get();

private:
    DLChain<PCIDevice> devices;
    
    DLChain<PCIDeviceDriver> drivers;

    static PCIManager instance;

};