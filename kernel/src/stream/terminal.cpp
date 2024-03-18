#include "terminal.hpp"

u32 TerminalOutput::get_cursor_x() {
    return cursorx;
}

u32 TerminalOutput::get_cursor_y() {
    return cursory;
}

void TerminalOutput::set_cursor(u32 x, u32 y) {
    set_cursor_raw(x, y);
    cursorx = x;
    cursory = y;

    if (y >= get_height())
        scroll(get_height() - y + 1);
}

void TerminalOutput::set_cursor_x(u32 v) {
    set_cursor(v, cursory);
}

void TerminalOutput::set_cursor_y(u32 v) {
    set_cursor(cursorx, v);
}

void TerminalOutput::set_bg(u8 color) {
    bg = color;
}

void TerminalOutput::set_fg(u8 color) {
    fg = color;
}

void TerminalOutput::set_color(u8 bg, u8 fg) {
    set_bg(bg);
    set_fg(fg);
}
    
void TerminalOutput::put(char ch) {
    if (!guide_cursor()) {
        put_raw(ch, bg, fg);
        return;
    }

    if (ch != '\t' && ch != '\n')
        put_raw(ch, bg, fg);

    if (ch == '\t') {
        cursorx = ((cursorx >> 3) + 1) << 3;
    } else {
        cursorx++;
    }
    
    if (cursorx >= get_width() || ch == '\n') {
        cursorx = 0;
        cursory++;
    }

    set_cursor(cursorx, cursory);
}

void TerminalOutput::write(const char* string, usize len) {
    for (usize i = 0; i < len; i++) {
        put(string[i]);
    }
}

void TerminalOutput::write(const char* string) {
    while (*string) {
        put(*(string++));
    }
}

void TerminalOutput::clear(u8 bg) {
    fill(' ', bg, fg);
}

void TerminalOutput::scroll(u32 lines) {
    scroll_raw(lines, bg, fg);

    set_cursor(cursorx, max(cursory - lines, (u32)0));
}