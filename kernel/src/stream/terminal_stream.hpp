#pragma once

#include <common.h>

#include "terminal.hpp"
#include "stream.hpp"

class TerminalOutputStream : public OutputStream {
public:
    void set_terminal(TerminalOutput* target);

    isize write(const void* data, usize size) override;

    inline u32 get_cursor_x() {
        if (target == nullptr) return 0;
        return target->get_cursor_x();
    }

    inline u32 get_cursor_y() {
        if (target == nullptr) return 0;
        return target->get_cursor_y();
    }

    inline void set_cursor(u32 x, u32 y) {
        if (target == nullptr) return;
        return target->set_cursor(x, y);
    }

    inline void set_cursor_x(u32 v) {
        if (target == nullptr) return;
        return target->set_cursor_x(v);
    }

    inline void set_cursor_y(u32 v) {
        if (target == nullptr) return;
        return target->set_cursor_y(v);
    }

    inline void set_bg(u8 color) {
        if (target == nullptr) return;
        return target->set_bg(color);
    }

    inline void set_fg(u8 color) {
        if (target == nullptr) return;
        return target->set_fg(color);
    }

    inline void set_color(u8 bg, u8 fg) {
        if (target == nullptr) return;
        return target->set_color(bg, fg);
    }
    
    inline void put(char ch) {
        if (target == nullptr) return;
        return target->put(ch);
    }

    inline void write(const char* string, usize len) {
        if (target == nullptr) return;
        return target->write(string, len);
    }

    inline void write(const char* string) {
        if (target == nullptr) return;
        return target->write(string);
    }

    inline void clear(u8 bg = -1) {
        if (target == nullptr) return;
        return target->clear(bg);
    }

    inline void scroll(u32 lines) {
        if (target == nullptr) return;
        return target->scroll(lines);
    }

    inline u32 get_width() const {
        if (target == nullptr) return 0;
        return target->get_width();
    }

    inline u32 get_height() const {
        if (target == nullptr) return 0;
        return target->get_height();
    }

    inline void set_cursor_visible(bool visible) const {
        if (target == nullptr) return;
        return target->set_cursor_visible(visible);
    }

private:
    TerminalOutput* target = nullptr;

};
