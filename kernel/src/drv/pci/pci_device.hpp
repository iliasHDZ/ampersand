#pragma once

#include <common.h>
#include <common/chain.hpp>

class PCIManager;

struct PACKED_STRUCT pci_general_device_config {
    u16 vendor_id;
    u16 device_id;
    u16 command;
    u16 status;

    u8 revision_id;
    u8 prog_if;
    u8 subclass;
    u8 class_code;
    u8 cache_line_size;
    u8 latency_timer;
    u8 header_type;
    u8 bist;

    u32 base_addresses[6];

    u32 cardbus_cis_pointer;

    u16 subsystemVendor_id;
    u16 subsystem_id;

    u32 exp_rom_base_address;
    u8 capabilities_pointer;

    u8 reserved[7];

    u8 interrupt_line;
    u8 interrupt_pin;
    u8 min_grant;
    u8 max_latency;
};

class PCIDevice : public DLChainItem {
private:
    inline PCIDevice(u8 bus, u8 device, u8 func)
        : bus_num(bus), device_num(device), func_num(func) {}

public:
    inline u32 get_bus_num()    const { return bus_num; };
    inline u32 get_device_num() const { return device_num; };
    inline u32 get_func_num()   const { return func_num; };

    inline u16 vendor_id() const { return gdev_config.vendor_id; };
    inline u16 device_id() const { return gdev_config.device_id; };

    inline u8 revision_id() const { return gdev_config.revision_id; };
    inline u8 prog_if()     const { return gdev_config.prog_if; };
    inline u8 subclass()    const { return gdev_config.subclass; };
    inline u8 class_code()  const { return gdev_config.class_code; };

    inline u8 header_type() const { return gdev_config.header_type; };

    inline u32 bar(u8 idx) const {
        if (idx >= 6) return 0;
        return gdev_config.base_addresses[idx];
    }

private:
    u8 bus_num;
    u8 device_num;
    u8 func_num;

    union {
        pci_general_device_config gdev_config;
        u32 config[16];
    };

    friend class PCIManager;
};