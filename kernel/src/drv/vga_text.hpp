#pragma once

#include <stream/terminal.hpp>
#include <fd/blkdev.hpp>

class VGATextDriver : public TerminalOutput {
public:
    u32 get_rows() const override;

    u32 get_columns() const override;

    void fill(char ch, u8 bg, u8 fg) override;

    void set_cursor_visible(bool visible) override;

    void put_raw(char ch, u8 bg, u8 fg) override;
    
    void set_cursor_raw(u32 x, u32 y) override;

    void scroll_raw(u32 lines, u8 bg, u8 fg) override;

public:
    static void init();

    static VGATextDriver* get();
    
protected:
    bool guide_cursor() override;

private:
    usize index;

};

class VGABlockDevice : public MemoryBlockDevice {
public:
    void* get_address() override;

    u64 get_size() override;

    usize ioctl_count(isize request) override;

    isize ioctl(isize request, usize* args) override;

public:
    static void init();

private:
    i32 cursor_x = 0;
    u32 cursor_y = 0;
};