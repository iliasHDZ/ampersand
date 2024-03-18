#include "ide.hpp"

#include <fs/devfs.hpp>
#include <arch/arch.hpp>
#include <logger.hpp>

/* Defines just taken from https://wiki.osdev.org/PCI_IDE_Controller */
#define ATA_SR_BSY   0x80    // Busy
#define ATA_SR_DRDY  0x40    // Drive ready
#define ATA_SR_DF    0x20    // Drive write fault
#define ATA_SR_DSC   0x10    // Drive seek complete
#define ATA_SR_DRQ   0x08    // Data request ready
#define ATA_SR_CORR  0x04    // Corrected data
#define ATA_SR_IDX   0x02    // Index
#define ATA_SR_ERR   0x01    // Error

#define ATA_ER_BBK   0x80    // Bad block
#define ATA_ER_UNC   0x40    // Uncorrectable data
#define ATA_ER_MC    0x20    // Media changed
#define ATA_ER_IDNF  0x10    // ID mark not found
#define ATA_ER_MCR   0x08    // Media change request
#define ATA_ER_ABRT  0x04    // Command aborted
#define ATA_ER_TK0NF 0x02    // Track 0 not found
#define ATA_ER_AMNF  0x01    // No address mark

#define ATA_CMD_READ_PIO        0x20
#define ATA_CMD_READ_PIO_EXT    0x24
#define ATA_CMD_READ_DMA        0xC8
#define ATA_CMD_READ_DMA_EXT    0x25
#define ATA_CMD_WRITE_PIO       0x30
#define ATA_CMD_WRITE_PIO_EXT   0x34
#define ATA_CMD_WRITE_DMA       0xCA
#define ATA_CMD_WRITE_DMA_EXT   0x35
#define ATA_CMD_CACHE_FLUSH     0xE7
#define ATA_CMD_CACHE_FLUSH_EXT 0xEA
#define ATA_CMD_PACKET          0xA0
#define ATA_CMD_IDENTIFY_PACKET 0xA1
#define ATA_CMD_IDENTIFY        0xEC

#define ATAPI_CMD_READ  0xA8
#define ATAPI_CMD_EJECT 0x1B

#define ATA_IDENT_DEVICETYPE   0
#define ATA_IDENT_CYLINDERS    2
#define ATA_IDENT_HEADS        6
#define ATA_IDENT_SECTORS      12
#define ATA_IDENT_SERIAL       20
#define ATA_IDENT_MODEL        54
#define ATA_IDENT_CAPABILITIES 98
#define ATA_IDENT_FIELDVALID   106
#define ATA_IDENT_MAX_LBA      120
#define ATA_IDENT_COMMANDSETS  164
#define ATA_IDENT_MAX_LBA_EXT  200

#define IDE_ATA        0x00
#define IDE_ATAPI      0x01
 
#define ATA_MASTER     0x00
#define ATA_SLAVE      0x01

#define ATA_REG_DATA       0x00
#define ATA_REG_ERROR      0x01
#define ATA_REG_FEATURES   0x01
#define ATA_REG_SECCOUNT0  0x02
#define ATA_REG_LBA0       0x03
#define ATA_REG_LBA1       0x04
#define ATA_REG_LBA2       0x05
#define ATA_REG_HDDEVSEL   0x06
#define ATA_REG_COMMAND    0x07
#define ATA_REG_STATUS     0x07
#define ATA_REG_SECCOUNT1  0x08
#define ATA_REG_LBA3       0x09
#define ATA_REG_LBA4       0x0A
#define ATA_REG_LBA5       0x0B
#define ATA_REG_CONTROL    0x0C
#define ATA_REG_ALTSTATUS  0x0C
#define ATA_REG_DEVADDRESS 0x0D

// Channels:
#define ATA_PRIMARY      0x00
#define ATA_SECONDARY    0x01
 
// Directions:
#define ATA_READ     0x00
#define ATA_WRITE    0x01

// To be implemented somewhere else sometime
static void sleep(u32 ms) {}

static u8 ide_buffer[2048] = {0};

u8 IDEDevice::get_block_size_bits() const {
    return BLOCK_SIZE_512B;
}

u64 IDEDevice::get_size() {
    return size * 512;
}

u64 IDEDevice::read_blocks(void* buf, u64 block_offset, u64 block_count) {
    if (block_offset >= size) return 0;
    if (block_count == 0) return 0;

    block_count = min(block_count, size - block_offset);

    if (channel->read_sectors(is_slave, block_offset, block_count, buf))
        return block_count;
    else
        return 0;
}

u64 IDEDevice::write_blocks(void* buf, u64 block_offset, u64 block_count) {
    if (block_offset >= size) return 0;
    if (block_count == 0) return 0;

    block_count = min(block_count, size - block_offset);

    if (channel->write_sectors(is_slave, block_offset, block_count, buf))
        return block_count;
    else
        return 0;
}

static u32 ide_device_counter = 0;

void IDEChannel::init() {
    write(ATA_REG_CONTROL, 2);

    for (u8 dev = 0; dev < 2; dev++) {
        u8 status = 0, err = 0;
        u8 type = IDE_ATA;

        devices[dev].present = false;

        select_drive(dev);
        sleep(1);

        write(ATA_REG_COMMAND, ATA_CMD_IDENTIFY);
        sleep(1);

        if (read(ATA_REG_STATUS) == 0) continue;

        while (1) {
            status = read(ATA_REG_STATUS);
            
            if (status & ATA_SR_ERR) {
                err = 1;
                break;
            }

            if (!(status & ATA_SR_BSY) && (status & ATA_SR_DRQ))
                break;
        }

        if (err != 0) {
            u8 cl = read(ATA_REG_LBA1);
            u8 ch = read(ATA_REG_LBA2);
 
            if (cl == 0x14 && ch == 0xEB)
                type = IDE_ATAPI;
            else if (cl == 0x69 && ch == 0x96)
                type = IDE_ATAPI;
            else
                continue;
 
            write(ATA_REG_COMMAND, ATA_CMD_IDENTIFY_PACKET);
            sleep(1);
            for(int i = 0; i < 4; i++)
                read(ATA_REG_ALTSTATUS);
        }

        read_buffer(ATA_REG_DATA, (u32*)ide_buffer, 128);

        devices[dev].present      = true;
        devices[dev].type         = type;
        devices[dev].channel      = this;
        devices[dev].is_slave     = dev;
        devices[dev].signature    = *((u16 *)(ide_buffer + ATA_IDENT_DEVICETYPE));
        devices[dev].capabilities = *((u16 *)(ide_buffer + ATA_IDENT_CAPABILITIES));
        devices[dev].command_sets = *((u32 *)(ide_buffer + ATA_IDENT_COMMANDSETS));

        if (devices[dev].command_sets & (1 << 26))
            devices[dev].size = *((u32 *)(ide_buffer + ATA_IDENT_MAX_LBA_EXT));
        else
            devices[dev].size = *((u32 *)(ide_buffer + ATA_IDENT_MAX_LBA));

        for (usize k = 0; k < 40; k += 2) {
            devices[dev].model[k + 0] = ide_buffer[ATA_IDENT_MODEL + k + 1];
            devices[dev].model[k + 1] = ide_buffer[ATA_IDENT_MODEL + k + 0];
        }
        
        devices[dev].model[40] = 0;

        for (usize i = 39; i != 0; i--) {
            if (devices[dev].model[i] != ' ')
                break;

            devices[dev].model[i] = 0;
        }

        const char* type_str = type ? "ATAPI" : "ATA";
        const char* dev_str  = dev ? "Slave" : "Master";
        const char* chnl_str = channel_num ? "Secondary" : "Primary";
    
        auto o = Log::INFO("IDEChannel");
        o << chnl_str << ' ' << dev_str << ' ' << type_str << " device: ";
        o.write_size((u64)(devices[dev].size) * 512);
        o << " \'" << devices[dev].model << "\'\n";
    }

    write(ATA_REG_CONTROL, 0);
    kthread_create(run_manager_thread, (void*)this);

    char dev_name[] = "sd_";

    for (u32 i = 0; i < 2; i++) {
        if (!devices[i].present) continue;

        dev_name[2] = 'a' + (ide_device_counter++);

        DevFileSystem::get()->add_block_device(&devices[i], dev_name);
    }
}

static ThreadSignal ide_execute_commands_signal;
static ThreadSignal ide_command_finished_signal;

bool IDEChannel::read_sectors(u8 dev, u64 lba, u32 count, void* buffer) {
    while (!command_queue.can_enqueue())
        ;

    IDEAccessCommand cmd = {
        .direction = IDEAccessCommand::READ,
        .drive = dev,
        .lba = lba,
        .numsects = count,
        .buffer = buffer
    };

    command_queue.enqueue(&cmd);

    kthread_emit(&ide_execute_commands_signal);

    while (!cmd.finished)
        kthread_await(&ide_command_finished_signal);

    if (cmd.err) {
        Log::ERR("IDEChannel") << Out::dec() << "Failed to read from " << (channel_num ? "SECONDARY" : "PRIMARY") << ' ' << dev << ":\n";
        Log::ERR("IDEChannel") << "  - " << cmd.err << '\n';
        return false;
    }

    return true;
}

bool IDEChannel::write_sectors(u8 dev, u64 lba, u32 count, void* buffer) {
    while (!command_queue.can_enqueue())
        ;

    IDEAccessCommand cmd = {
        .direction = IDEAccessCommand::WRITE,
        .drive = dev,
        .lba = lba,
        .numsects = count,
        .buffer = buffer
    };

    command_queue.enqueue(&cmd);

    kthread_emit(&ide_execute_commands_signal);

    while (!cmd.finished)
        kthread_await(&ide_command_finished_signal);

    if (cmd.err) {
        Log::ERR("IDEChannel") << Out::dec() << "Failed to write to " << (channel_num ? "SECONDARY" : "PRIMARY") << ' ' << dev << ": \n";
        Log::ERR("IDEChannel") << "  - " << cmd.err << '\n';
        return false;
    }

    return true;
}

void IDEChannel::select_drive(u8 dev, u8 use_lba, u8 head) {
    write(ATA_REG_HDDEVSEL, 0xA0 | (use_lba << 6) | (dev << 4) | head);
}

IDEAddressMode IDEChannel::set_access_section(u8 dev, u64 lba, u8 numsects) {
    IDEAddressMode mode;
    u8 lba_io[6];
    u8 head, sect;
    u16 cyl;

    write(ATA_REG_CONTROL, 2);
    
    if (lba >= 0x10000000) {
        mode      = IDEAddressMode::LBA48;
        lba_io[0] = (lba & 0x0000000000FF) >> 0;
        lba_io[1] = (lba & 0x00000000FF00) >> 8;
        lba_io[2] = (lba & 0x000000FF0000) >> 16;
        lba_io[3] = (lba & 0x0000FF000000) >> 24;
        lba_io[4] = (lba & 0x00FF00000000) >> 32;
        lba_io[5] = (lba & 0xFF0000000000) >> 40;
        head      = 0;
    } else if (devices[dev].capabilities & 0x200)  {
        mode      = IDEAddressMode::LBA28;
        lba_io[0] = (lba & 0x00000FF) >> 0;
        lba_io[1] = (lba & 0x000FF00) >> 8;
        lba_io[2] = (lba & 0x0FF0000) >> 16;
        lba_io[3] = 0;
        lba_io[4] = 0;
        lba_io[5] = 0;
        head      = (lba & 0xF000000) >> 24;
    } else {
        mode      = IDEAddressMode::CHS;
        sect      = (lba % 63) + 1;
        cyl       = (lba + 1  - sect) / (16 * 63);
        lba_io[0] = sect;
        lba_io[1] = (cyl >> 0) & 0xFF;
        lba_io[2] = (cyl >> 8) & 0xFF;
        lba_io[3] = 0;
        lba_io[4] = 0;
        lba_io[5] = 0;
        head      = (lba + 1  - sect) % (16 * 63) / (63);
    }

    select_drive(dev, mode != IDEAddressMode::CHS, head);

    if (mode == IDEAddressMode::LBA48) {
        write(ATA_REG_SECCOUNT1, 0);
        write(ATA_REG_LBA3, lba_io[3]);
        write(ATA_REG_LBA4, lba_io[4]);
        write(ATA_REG_LBA5, lba_io[5]);
    }
    write(ATA_REG_SECCOUNT0, numsects);
    write(ATA_REG_LBA0, lba_io[0]);
    write(ATA_REG_LBA1, lba_io[1]);
    write(ATA_REG_LBA2, lba_io[2]);

    return mode;
}

void IDEChannel::send_read_command(IDEAddressMode addr_mode, bool use_dma) {
    u8 cmd;
    
    if (addr_mode == IDEAddressMode::CHS   && !use_dma) cmd = ATA_CMD_READ_PIO;
    if (addr_mode == IDEAddressMode::LBA28 && !use_dma) cmd = ATA_CMD_READ_PIO;   
    if (addr_mode == IDEAddressMode::LBA48 && !use_dma) cmd = ATA_CMD_READ_PIO_EXT;   
    if (addr_mode == IDEAddressMode::CHS   &&  use_dma) cmd = ATA_CMD_READ_DMA;
    if (addr_mode == IDEAddressMode::LBA28 &&  use_dma) cmd = ATA_CMD_READ_DMA;
    if (addr_mode == IDEAddressMode::LBA48 &&  use_dma) cmd = ATA_CMD_READ_DMA_EXT;
    
    write(ATA_REG_COMMAND, cmd);
}

void IDEChannel::send_write_command(IDEAddressMode addr_mode, bool use_dma) {
    u8 cmd;

    if (addr_mode == IDEAddressMode::CHS   && !use_dma) cmd = ATA_CMD_WRITE_PIO;
    if (addr_mode == IDEAddressMode::LBA28 && !use_dma) cmd = ATA_CMD_WRITE_PIO;
    if (addr_mode == IDEAddressMode::LBA48 && !use_dma) cmd = ATA_CMD_WRITE_PIO_EXT;
    if (addr_mode == IDEAddressMode::CHS   &&  use_dma) cmd = ATA_CMD_WRITE_DMA;
    if (addr_mode == IDEAddressMode::LBA28 &&  use_dma) cmd = ATA_CMD_WRITE_DMA;
    if (addr_mode == IDEAddressMode::LBA48 &&  use_dma) cmd = ATA_CMD_WRITE_DMA_EXT;

    write(ATA_REG_COMMAND, cmd);
}

const char* IDEChannel::polling(unsigned int advanced_check) {
    for (int i = 0; i < 4; i++)
        read(ATA_REG_ALTSTATUS);

    while (read(ATA_REG_STATUS) & ATA_SR_BSY)
        ;
    
    if (advanced_check) {
        unsigned char state = read(ATA_REG_STATUS);

        if (state & ATA_SR_ERR)
            return "General Error";
        
        if (state & ATA_SR_DF)
            return "Drive Fault";
            
        if ((state & ATA_SR_DRQ) == 0)
            return "DRQ should be set";
    }
    
    return 0;
}

const char* IDEChannel::read_sectors_raw(u8 dev, u64 lba, u8 count, void* buffer) {
    while (read(ATA_REG_STATUS) & ATA_SR_BSY)
        ;
    
    IDEAddressMode mode = set_access_section(dev, lba, count);
    send_read_command(mode);

    u16* buf = (u16*)buffer;

    for (u32 sec = 0; sec < count; sec++) {
        const char* err = polling(1);
        if (err) return err;

        for (u32 i = 0; i < 256; i++)
            *(buf++) = arch_inw(base_port);
    }

    return nullptr;
}

const char* IDEChannel::write_sectors_raw(u8 dev, u64 lba, u8 count, void* buffer) {
    while (read(ATA_REG_STATUS) & ATA_SR_BSY)
        ;
    
    IDEAddressMode mode = set_access_section(dev, lba, count);
    send_write_command(mode);

    u16* buf = (u16*)buffer;

    for (u32 sec = 0; sec < count; sec++) {
        const char* err = polling(1);
        if (err) return err;

        for (u32 i = 0; i < 256; i++)
            arch_outw(base_port, *(buf++));
    }

    return nullptr;
}

u8 IDEChannel::read(u8 reg) {
    u8 res;
    if (reg > 0x07 && reg < 0x0C)
        write(ATA_REG_CONTROL, 0x80);
    
    if (reg < 0x08)
        res = arch_inb(base_port + reg - 0x00);
    else if (reg < 0x0C)
        res = arch_inb(base_port + reg - 0x06);
    else if (reg < 0x0E)
        res = arch_inb(ctrl_port + reg - 0x0A);
    else if (reg < 0x16)
        res = arch_inb(bmst_port + reg - 0x0E);
    
    if (reg > 0x07 && reg < 0x0C)
        write(ATA_REG_CONTROL, 0);
    
    return res;
}

void IDEChannel::write(u8 reg, u8 val) {
    if (reg > 0x07 && reg < 0x0C)
        write(ATA_REG_CONTROL, 0x80);
    
    if (reg < 0x08)
        arch_outb(base_port + reg - 0x00, val);
    else if (reg < 0x0C)
        arch_outb(base_port + reg - 0x06, val);
    else if (reg < 0x0E)
        arch_outb(ctrl_port + reg - 0x0A, val);
    else if (reg < 0x16)
        arch_outb(bmst_port + reg - 0x0E, val);
    
    if (reg > 0x07 && reg < 0x0C)
        write(ATA_REG_CONTROL, 0);
}

static void inl_buffer(u16 port, u32* buffer, usize count) {
    while (count) {
        *(buffer++) = arch_inl(port);
        count--;
    }
}

void IDEChannel::read_buffer(u8 reg, u32* dst, usize count) {
    if (reg > 0x07 && reg < 0x0C)
        write(ATA_REG_CONTROL, 0x80);
    
    if (reg < 0x08)
        inl_buffer(base_port + reg - 0x00, dst, count);
    else if (reg < 0x0C)
        inl_buffer(base_port + reg - 0x06, dst, count);
    else if (reg < 0x0E)
        inl_buffer(ctrl_port + reg - 0x0A, dst, count);
    else if (reg < 0x16)
        inl_buffer(bmst_port + reg - 0x0E, dst, count);
    
    if (reg > 0x07 && reg < 0x0C)
        write(ATA_REG_CONTROL, 0);
}

void IDEChannel::manager_thread() {
    while (true) {
        kthread_await(&ide_execute_commands_signal);

        if (!command_queue.can_dequeue())
            continue;

        IDEAccessCommand* cmd = command_queue.dequeue();

        usize count = cmd->numsects;
        usize lba   = cmd->lba;
        u8* buffer  = (u8*)cmd->buffer;

        const char* err = nullptr;

        while (count > 0) {
            u8 seccount = (u8)min(count, (usize)255);

            if (cmd->direction == IDEAccessCommand::READ)
                err = read_sectors_raw(cmd->drive, lba, seccount, (void*)buffer);
            else
                err = write_sectors_raw(cmd->drive, lba, seccount, (void*)buffer);

            if (err) break;

            lba += seccount;
            buffer += 512 * seccount;

            count -= seccount;
        }

        cmd->err = err;
        cmd->finished = true;

        kthread_emit(&ide_command_finished_signal);
    }
}

void IDEChannel::run_manager_thread(void* channel) {
    if (channel == nullptr) {
        panic("IDEChannel: The manager thread recieved a nullptr param");
        return;
    }

    ((IDEChannel*)channel)->manager_thread();
}

IDEController::IDEController(PCIDevice* pci_device)
    : PCIDeviceDriver(pci_device) {}

bool IDEController::init() {
    if (pci_device->class_code() != 1 || pci_device->subclass() != 1)
        return false;

    Log::INFO("IDEController") << "Initializing controller...\n";

    u32 bar0 = pci_device->bar(0);
    u32 bar1 = pci_device->bar(1);
    u32 bar2 = pci_device->bar(2);
    u32 bar3 = pci_device->bar(3);
    u32 bar4 = pci_device->bar(4);

    channels[0].base_port = bar0 ? (bar0 & 0xfffffffc) : 0x1f0;
    channels[0].ctrl_port = bar1 ? (bar1 & 0xfffffffc) : 0x3f6;
    channels[1].base_port = bar2 ? (bar2 & 0xfffffffc) : 0x170;
    channels[1].ctrl_port = bar3 ? (bar3 & 0xfffffffc) : 0x376;

    channels[0].bmst_port = (bar4 & 0xfffffffc) + 0;
    channels[1].bmst_port = (bar4 & 0xfffffffc) + 8;

    channels[0].init();
    channels[1].init();
    
    return true;
}