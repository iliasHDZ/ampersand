#pragma once

#include <common.h>

#define TERM_BLACK        0x0
#define TERM_BLUE         0x1
#define TERM_GREEN        0x2
#define TERM_CYAN         0x3
#define TERM_RED          0x4
#define TERM_PURPLE       0x5
#define TERM_BROWN        0x6
#define TERM_GRAY         0x7
#define TERM_DARK_GRAY    0x8
#define TERM_LIGHT_BLUE   0x9
#define TERM_LIGHT_GREEN  0xA
#define TERM_LIGHT_CYAN   0xB
#define TERM_LIGHT_RED    0xC
#define TERM_LIGHT_PURPLE 0xD
#define TERM_YELLOW       0xE
#define TERM_WHITE        0xF

class TerminalOutput {
public:
    virtual u32 get_rows() const = 0;

    virtual u32 get_columns() const = 0;

    virtual void fill(char ch, u8 bg, u8 fg) = 0;

    virtual void set_cursor_visible(bool visible) = 0;

protected:
    virtual bool guide_cursor() = 0;

    virtual void put_raw(char ch, u8 bg, u8 fg) = 0;
    
    virtual void set_cursor_raw(u32 x, u32 y) = 0;

    virtual void scroll_raw(u32 lines, u8 bg, u8 fg) = 0;

public:
    u32 get_cursor_x();

    u32 get_cursor_y();

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

    inline u32 get_width() const {
        return get_columns();
    }

    inline u32 get_height() const {
        return get_rows();
    }

private:
    u32 cursorx = 0;
    u32 cursory = 0;

    u8 bg = TERM_BLACK;
    u8 fg = TERM_GRAY;
};