#pragma once

#include "pci/pci.hpp"

#include <fd/block_transfer.hpp>
#include <fd/blkdev.hpp>
#include <data/queue.hpp>
#include <data/thread.hpp>

class IDEChannel;

enum class IDEAddressMode {
    CHS,
    LBA28,
    LBA48
};

class IDEDevice : public BlockTransferFileDescription<BlockDevice> {
public:
    inline IDEDevice(IDEChannel* channel, u8 is_slave)
        : channel(channel) {}

    u8 get_block_size_bits() const override;

    u64 get_size() override;

    u64 read_blocks(void* buf, u64 block_offset, u64 block_count) override;

    u64 write_blocks(void* buf, u64 block_offset, u64 block_count) override;

public:
    bool present = false;
    IDEChannel* channel;
    u8 is_slave;

    u16 type;
    u16 signature;
    u16 capabilities;
    u32 command_sets;
    u32 size;

    char model[41];
};

class IDEChannel {
public:
    inline IDEChannel(u8 channel_num)
        : channel_num(channel_num) {}

    void init();

    bool read_sectors(u8 dev, u64 lba, u32 count, void* buffer);

    bool write_sectors(u8 dev, u64 lba, u32 count, void* buffer);

private:
    void select_drive(u8 dev, u8 use_lba = 0, u8 head = 0);

    const char* polling(unsigned int advanced_check);
    
    IDEAddressMode set_access_section(u8 dev, u64 lba, u8 numsects);

    void send_read_command(IDEAddressMode addr_mode, bool use_dma = false);

    void send_write_command(IDEAddressMode addr_mode, bool use_dma = false);

    u8 read(u8 reg);
    
    void write(u8 reg, u8 val);
    
    void read_buffer(u8 reg, u32* dst, usize count);

public:
    u8 channel_num;

    u16 base_port;
    u16 ctrl_port;
    u16 bmst_port;

    Mutex mutex;

    IDEDevice devices[2] = { IDEDevice(this, 0), IDEDevice(this, 1) };
};

class IDEController : public PCIDeviceDriver {
public:
    IDEController(PCIDevice* pci_device);

    bool init() override;

private:
    IDEChannel channels[2] = { IDEChannel(0), IDEChannel(1) };
};