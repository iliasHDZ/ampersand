#pragma once

#include "terminal.hpp"

#define ANSI_BEL 0x07
#define ANSI_BS  0x08
#define ANSI_HT  0x09
#define ANSI_LF  0x0A
#define ANSI_FF  0x0C
#define ANSI_CR  0x0D
#define ANSI_ESC 0x1B

class ANSIParser {
public:
    ANSIParser(Terminal* term);

    void write(char* buf, int size);

    void put(char ch);

private:
    Terminal* term;
    char bg = 0, fg = 0xf;
};