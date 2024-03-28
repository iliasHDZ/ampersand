#pragma once

#include <common.h>

#include "terminal.hpp"
#include "stream.hpp"

#define MAX_TERM_OUTPUT_TARGETS 4

class TerminalOutputStream : public OutputStream {
public:
    void add_terminal(TerminalOutput* target);

    void enable_terminal(TerminalOutput* target);

    void disable_terminal(TerminalOutput* target);

    isize write(const void* data, usize size) override;

    void set_cursor(u32 x, u32 y);

    void set_cursor_x(u32 v);

    void set_cursor_y(u32 v);

    void set_bg(u8 color);

    void set_fg(u8 color);

    void set_color(u8 bg, u8 fg);
    
    void put(char ch);

    void write(const char* string, usize len);

    void write(const char* string);

    void clear(u8 bg = -1);

    void scroll(u32 lines);

    void set_cursor_visible(bool visible) const;

private:
    TerminalOutput* targets[MAX_TERM_OUTPUT_TARGETS];
    bool targets_enabled[MAX_TERM_OUTPUT_TARGETS];
    usize target_count = 0;
};
