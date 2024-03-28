#include "terminal_stream.hpp"

void TerminalOutputStream::add_terminal(TerminalOutput* target) {
    if (target_count >= MAX_TERM_OUTPUT_TARGETS)
        panic("TerminalOutputStream: target_count >= MAX_TERM_OUTPUT_TARGETS");

    usize id = target_count++;
    targets[id] = target;
    targets_enabled[id] = true;
}

void TerminalOutputStream::enable_terminal(TerminalOutput* target) {
    for (usize i = 0; i < target_count; i++)
        if (targets[i] == target) {
            targets_enabled[i] = true;
        }
}

void TerminalOutputStream::disable_terminal(TerminalOutput* target) {
    for (usize i = 0; i < target_count; i++)
        if (targets[i] == target) {
            targets_enabled[i] = false;
        }
}

isize TerminalOutputStream::write(const void* data, usize size) {
    for (usize i = 0; i < target_count; i++)
        if (targets_enabled[i])
            targets[i]->write((const char*)data, size);
    return size;
}

void TerminalOutputStream::set_cursor(u32 x, u32 y) {
    for (usize i = 0; i < target_count; i++)
        if (targets_enabled[i])
            targets[i]->set_cursor(x, y);
}

void TerminalOutputStream::set_cursor_x(u32 v) {
    for (usize i = 0; i < target_count; i++)
        if (targets_enabled[i])
            targets[i]->set_cursor_x(v);
}

void TerminalOutputStream::set_cursor_y(u32 v) {
    for (usize i = 0; i < target_count; i++)
        if (targets_enabled[i])
            targets[i]->set_cursor_y(v);
}

void TerminalOutputStream::set_bg(u8 color) {
    for (usize i = 0; i < target_count; i++)
        if (targets_enabled[i])
            targets[i]->set_bg(color);
}

void TerminalOutputStream::set_fg(u8 color) {
    for (usize i = 0; i < target_count; i++)
        if (targets_enabled[i])
            targets[i]->set_fg(color);
}

void TerminalOutputStream::set_color(u8 bg, u8 fg) {
    for (usize i = 0; i < target_count; i++)
        if (targets_enabled[i])
            targets[i]->set_color(bg, fg);
}

void TerminalOutputStream::put(char ch) {
    for (usize i = 0; i < target_count; i++)
        if (targets_enabled[i])
            targets[i]->put(ch);
}

void TerminalOutputStream::write(const char* string, usize len) {
    for (usize i = 0; i < target_count; i++)
        if (targets_enabled[i])
            targets[i]->write(string, len);
}

void TerminalOutputStream::write(const char* string) {
    for (usize i = 0; i < target_count; i++)
        if (targets_enabled[i])
            targets[i]->write(string);
}

void TerminalOutputStream::clear(u8 bg) {
    for (usize i = 0; i < target_count; i++)
        if (targets_enabled[i])
            targets[i]->clear(bg);
}

void TerminalOutputStream::scroll(u32 lines) {
    for (usize i = 0; i < target_count; i++)
        if (targets_enabled[i])
            targets[i]->scroll(lines);
}

void TerminalOutputStream::set_cursor_visible(bool visible) const {
    for (usize i = 0; i < target_count; i++)
        if (targets_enabled[i])
            targets[i]->set_cursor_visible(visible);
}