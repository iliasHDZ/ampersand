#include "debugout.hpp"

#include <arch/arch.hpp>

static void dbgput(u8 ch) {
    arch_outb(0xe9, ch);
}

static void dbgstr(const char* ch) {
    while (*ch)
        dbgput(*(ch++));
}

static void dbgcol(u8 bg, u8 fg) {
    dbgstr("\e[");

    switch (bg) {
    default:
    case TERM_BLACK:  dbgstr("40"); break;
    case TERM_RED:    dbgstr("41"); break;
    case TERM_GREEN:  dbgstr("42"); break;
    case TERM_BROWN:  dbgstr("43"); break;
    case TERM_BLUE:   dbgstr("44"); break;
    case TERM_PURPLE: dbgstr("45"); break;
    case TERM_CYAN:   dbgstr("46"); break;
    case TERM_GRAY:   dbgstr("47"); break;
    case TERM_DARK_GRAY:    dbgstr("100"); break;
    case TERM_LIGHT_RED:    dbgstr("101"); break;
    case TERM_LIGHT_GREEN:  dbgstr("102"); break;
    case TERM_YELLOW:       dbgstr("103"); break;
    case TERM_LIGHT_BLUE:   dbgstr("104"); break;
    case TERM_LIGHT_PURPLE: dbgstr("105"); break;
    case TERM_LIGHT_CYAN:   dbgstr("106"); break;
    case TERM_WHITE:        dbgstr("107"); break;
    }

    dbgput(';');

    switch (fg) {
    case TERM_BLACK:  dbgstr("30"); break;
    case TERM_RED:    dbgstr("31"); break;
    case TERM_GREEN:  dbgstr("32"); break;
    case TERM_BROWN:  dbgstr("33"); break;
    case TERM_BLUE:   dbgstr("34"); break;
    case TERM_PURPLE: dbgstr("35"); break;
    case TERM_CYAN:   dbgstr("36"); break;
    case TERM_GRAY:   dbgstr("37"); break;
    case TERM_DARK_GRAY:    dbgstr("90"); break;
    case TERM_LIGHT_RED:    dbgstr("91"); break;
    case TERM_LIGHT_GREEN:  dbgstr("92"); break;
    case TERM_YELLOW:       dbgstr("93"); break;
    case TERM_LIGHT_BLUE:   dbgstr("94"); break;
    case TERM_LIGHT_PURPLE: dbgstr("95"); break;
    case TERM_LIGHT_CYAN:   dbgstr("96"); break;
    default:
    case TERM_WHITE:        dbgstr("97"); break;
    }

    dbgput('m');
}

u32 DebugTerminalOutput::get_rows() const {
    return 0;
}

u32 DebugTerminalOutput::get_columns() const {
    return 0;
}

void DebugTerminalOutput::fill(char ch, u8 bg, u8 fg) {
    set_out_color(bg, fg);
}

void DebugTerminalOutput::set_cursor_visible(bool visible) {
    
}

void DebugTerminalOutput::put_raw(char ch, u8 bg, u8 fg) {
    set_out_color(bg, fg);
    dbgput(ch);
}

void DebugTerminalOutput::set_cursor_raw(u32 x, u32 y) {

}

void DebugTerminalOutput::scroll_raw(u32 lines, u8 bg, u8 fg) {

}

bool DebugTerminalOutput::guide_cursor() {
    return false;
}

void DebugTerminalOutput::set_out_color(u8 bg, u8 fg) {
    if (cbg != bg || cfg != fg) {
        dbgcol(bg, fg);
        cbg = bg;
        cfg = fg;
    }
}