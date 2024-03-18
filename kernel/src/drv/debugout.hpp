#pragma once

#include "../stream/terminal.hpp"

class DebugTerminalOutput : public TerminalOutput {
public:
    inline DebugTerminalOutput() {}

    u32 get_rows() const override;

    u32 get_columns() const override;

    void fill(char ch, u8 bg, u8 fg) override;

    void set_cursor_visible(bool visible) override;

    void put_raw(char ch, u8 bg, u8 fg) override;
    
    void set_cursor_raw(u32 x, u32 y) override;

    void scroll_raw(u32 lines, u8 bg, u8 fg) override;

protected:
    bool guide_cursor() override;

private:
    void set_out_color(u8 bg, u8 fg);

private:
    u8 cfg, cbg;
};