#pragma once

#include "keys.hpp"

#define UNK 0

static keymapping keymappings_be[] = {
    { KEY_ONE      , '&', '1', '|' },
    { KEY_TWO      , UNK, '2', '@' },
    { KEY_THREE    , '"', '3', '#' },
    { KEY_FOUR     , '\'','4', '{' },
    { KEY_FIVE     , '(', '5', '[' },
    { KEY_SIX      , UNK, '6', '^' },
    { KEY_SEVEN    , UNK, '7' },
    { KEY_EIGHT    , '!', '8' },
    { KEY_NINE     , UNK, '9', '{' },
    { KEY_ZERO     , UNK, '0', '}' },
    { KEY_MINUS    , ')', UNK },
    { KEY_EQUAL    , '-', '_' },
    { KEY_Q        , 'a', 'A' },
    { KEY_W        , 'z', 'Z' },
    { KEY_E        , 'e', 'E' },
    { KEY_R        , 'r', 'R' },
    { KEY_T        , 't', 'T' },
    { KEY_Y        , 'y', 'Y' },
    { KEY_U        , 'u', 'U' },
    { KEY_I        , 'i', 'I' },
    { KEY_O        , 'o', 'O' },
    { KEY_P        , 'p', 'P' },
    { KEY_OPENBRACK, '^', UNK, '[' },
    { KEY_CLOSBRACK, '$', '*', ']' },
    { KEY_A        , 'q', 'Q' },
    { KEY_S        , 's', 'S' },
    { KEY_D        , 'd', 'D' },
    { KEY_F        , 'f', 'F' },
    { KEY_G        , 'g', 'G' },
    { KEY_H        , 'h', 'H' },
    { KEY_J        , 'j', 'J' },
    { KEY_K        , 'k', 'K' },
    { KEY_L        , 'l', 'L' },
    { KEY_SEMICOLON, 'm', 'M' },
    { KEY_QUOTE    , UNK, '%', UNK },
    { KEY_BACKTICK , UNK },
    { 0x56         , '<', '>', '\\' },
    { KEY_Z        , 'w', 'W' },
    { KEY_X        , 'x', 'X' },
    { KEY_C        , 'c', 'C' },
    { KEY_V        , 'v', 'V' },
    { KEY_B        , 'b', 'B' },
    { KEY_N        , 'n', 'N' },
    { KEY_M        , ',', '?' },
    { KEY_COMMA    , ';', '.' },
    { KEY_PERIOD   , ':', '/' },
    { KEY_SLASH    , '=', '+', '~' },
    { KEY_ASTERISK , '*' },
    { KEY_SPACE    , ' ' },
};

static keymap keymap_be = {
    keymappings_be,
    sizeof(keymappings_be) / sizeof(keymapping)
};