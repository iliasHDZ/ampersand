#include "ansi.hpp"

ANSIParser::ANSIParser(Terminal* term)
    : term(term) {}

void ANSIParser::write(char* buf, int size) {
    while (size) {
        put(*buf);
        buf++;
        size--;
    }
}

void ANSIParser::write(char* buf) {
    while (*buf) {
        put(*buf);
        buf++;
    }
}

void ANSIParser::put(char ch) {
    if (esc_enabled) {
        esc_code(ch);
        esc_enabled = false;
        return;
    }

    if (csi_enabled) {
        csi_char(ch);
        return;
    }
    
    int count;
    switch (ch) {
    case ANSI_BEL:
        // Make bell noise
        break;
    case ANSI_BS:
        cursor_back();
        break;
    case ANSI_HT:
        count = 8 - (cursor_x % 8);
        for (int i = 0; i < count; i++)
            cursor_forward();
        break;
    case ANSI_LF:
        cursor_x = 0;
        cursor_down();
        break;
    case ANSI_FF:
        cursor_x = 0;
        cursor_y = 0;
        update_cursor();
        break;
    case ANSI_CR:
        cursor_x = 0;
        update_cursor();
        break;
    case ANSI_ESC:
        esc_enabled = true;
        break;
    default:
        term->put(ch, bg, fg);
        cursor_forward();
        break;
    }
}

void ANSIParser::cursor_up(int n) {
    for (int i = 0; i < n; i++) {
        if (cursor_y > 0)
            cursor_y--;
    }

    update_cursor();
}

void ANSIParser::cursor_down(int n) {
    for (int i = 0; i < n; i++) {
        cursor_y++;

        if (cursor_y >= term->get_height()) {
            term->scroll(bg, fg);
            cursor_y--;
        }
    }

    update_cursor();
}

void ANSIParser::cursor_forward(int n) {
    for (int i = 0; i < n; i++) {
        cursor_x++;
        
        if (cursor_x >= term->get_width()) {
            cursor_x = 0;
            cursor_y++;
        }

        if (cursor_y >= term->get_height()) {
            term->scroll(bg, fg);
            cursor_y--;
        }
    }

    update_cursor();
}

void ANSIParser::cursor_back(int n) {
    for (int i = 0; i < n; i++) {
        if (cursor_x > 0)
            cursor_x--;
        else if (cursor_y > 0) {
            cursor_x = term->get_width() - 1;
            cursor_y--;
        }
    }

    update_cursor();
}

void ANSIParser::update_cursor() {
    term->set_cursor(cursor_x, cursor_y);
}

void ANSIParser::esc_code(char code) {
    if (code == '[') {
        for (int i = 0; i < 4; i++)
            csi_args[i] = -1;

        csi_arg_idx = 0;
        csi_enabled = true;
    }
}

void ANSIParser::csi_char(char ch) {
    if (ch >= '0' && ch <= '9') {
        if (csi_args[csi_arg_idx] == -1)
            csi_args[csi_arg_idx] = 0;
        
        csi_args[csi_arg_idx] = csi_args[csi_arg_idx] * 10 + (ch - '0');
        return;
    }

    if (ch == ';') {
        csi_arg_idx++;
        if (csi_arg_idx >= 4) {
            csi_enabled = false;
            return;
        }
    }

    if (ch >= 0x40 && ch <= 0x7E) {
        run_csi_sequence(ch);
        csi_enabled = false;
    }
}

void ANSIParser::csi_clear(int n, bool line) {
    int tw = term->get_width(), th = term->get_height();

    int count = 0;
    switch (n) {
    case 0:
        if (line)
            count = tw - cursor_x;
        else
            count = tw * th - (cursor_y * tw + cursor_x);
        break;
    case 1:
        if (line) {
            term->set_cursor(0, cursor_y);
            count = cursor_x;
        } else {
            term->set_cursor(0, 0);
            count = cursor_y * tw + cursor_x;
        }
        break;
    case 2:
        if (line) {
            term->set_cursor(0, cursor_y);
            count = tw;
        } else {
            term->set_cursor(0, 0);
            count = tw * th;
        }
        break;
    };

    for (int i = 0; i < count; i++)
        term->put(' ', bg, fg);

    update_cursor();
}

void ANSIParser::run_csi_sequence(char code) {
    int n = csi_args[0] == -1 ? 1 : csi_args[0];
    int m = csi_args[1] == -1 ? 1 : csi_args[1];

    switch (code) {
    case ANSI_CSI_CUU:
        cursor_up(n);
        break;
    case ANSI_CSI_CUD:
        cursor_down(n);
        break;
    case ANSI_CSI_CUF:
        cursor_forward(n);
        break;
    case ANSI_CSI_CUB:
        cursor_back(n);
        break;
    case ANSI_CSI_CNL:
        cursor_x = 0;
        cursor_down(n);
        break;
    case ANSI_CSI_CPL:
        cursor_x = 0;
        cursor_up(n);
        break;
    case ANSI_CSI_CHA:
        cursor_x = n;
        update_cursor();
        break;
    case ANSI_CSI_CUP:
    case ANSI_CSI_HVP:
        cursor_x = n;
        cursor_x = m;
        update_cursor();
        break;
    case ANSI_CSI_ED:
        if (csi_args[0] == -1)
            n = 0;
        csi_clear(n, false);
        break;
    case ANSI_CSI_EL:
        if (csi_args[0] == -1)
            n = 0;
        csi_clear(n, true);
        break;
    case ANSI_CSI_SU:
        for (int i = 0; i < n; i++)
            term->scroll(bg, fg);
        cursor_up(n);
        break;
    case ANSI_CSI_SD:
        break;
    case ANSI_CSI_SGR:
        if (csi_args[0] == -1)
            return;
        select_sgr(n);
        break;
    }
}

void ANSIParser::select_sgr(char code) {
    if (code >= 30 && code <= 37) {
        fg = code - 30;
        return;
    }
    
    if (code >= 40 && code <= 47) {
        bg = code - 40;
        return;
    }

    if (code >= 90 && code <= 97) {
        fg = code - 90 + 8;
        return;
    }
    
    if (code >= 100 && code <= 107) {
        bg = code - 100 + 8;
        return;
    }
}