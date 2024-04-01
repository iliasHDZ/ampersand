#pragma once

#include "terminal.hpp"

#define ANSI_BEL 0x07
#define ANSI_BS  0x08
#define ANSI_HT  0x09
#define ANSI_LF  0x0A
#define ANSI_FF  0x0C
#define ANSI_CR  0x0D
#define ANSI_ESC 0x1B

#define ANSI_CSI_CUU 'A'
#define ANSI_CSI_CUD 'B'
#define ANSI_CSI_CUF 'C'
#define ANSI_CSI_CUB 'D'
#define ANSI_CSI_CNL 'E'
#define ANSI_CSI_CPL 'F'
#define ANSI_CSI_CHA 'G'
#define ANSI_CSI_CUP 'H'
#define ANSI_CSI_ED  'J'
#define ANSI_CSI_EL  'K'
#define ANSI_CSI_SU  'S'
#define ANSI_CSI_SD  'T'
#define ANSI_CSI_HVP 'f'
#define ANSI_CSI_SGR 'm'
#define ANSI_CSI_AUX 'i'
#define ANSI_CSI_DSR 'n'

class ANSIParser {
public:
    ANSIParser(Terminal* term);

    void write(char* buf, int size);

    void write(char* buf);

    void put(char ch);

private:
    void cursor_up(int n = 1);
    
    void cursor_down(int n = 1);

    void cursor_forward(int n = 1);
    
    void cursor_back(int n = 1);

    void update_cursor();

    void esc_code(char code);

    void csi_char(char ch);

    void csi_clear(int n, bool line);

    void run_csi_sequence(char code);

    void select_sgr(char code);

private:
    Terminal* term;
    char bg = 0, fg = 0xf;
    int cursor_x = 0, cursor_y = 0;

    bool csi_enabled = false;
    bool esc_enabled = false;

    int csi_args[4];
    int csi_arg_idx = 0;
    int csi_argc = 0;
};