#pragma once

#define KEY_ESC       0x01
#define KEY_ONE       0x02
#define KEY_TWO       0x03
#define KEY_THREE     0x04
#define KEY_FOUR      0x05
#define KEY_FIVE      0x06
#define KEY_SIX       0x07
#define KEY_SEVEN     0x08
#define KEY_EIGHT     0x09
#define KEY_NINE      0x0a
#define KEY_ZERO      0x0b
#define KEY_MINUS     0x0c
#define KEY_EQUAL     0x0d
#define KEY_BACKSPACE 0x0e
#define KEY_TAB       0x0f
#define KEY_Q         0x10
#define KEY_W         0x11
#define KEY_E         0x12
#define KEY_R         0x13
#define KEY_T         0x14
#define KEY_Y         0x15
#define KEY_U         0x16
#define KEY_I         0x17
#define KEY_O         0x18
#define KEY_P         0x19
#define KEY_OPENBRACK 0x1a
#define KEY_CLOSBRACK 0x1b
#define KEY_ENTER     0x1c
#define KEY_LCTRL     0x1d
#define KEY_A         0x1e
#define KEY_S         0x1f
#define KEY_D         0x20
#define KEY_F         0x21
#define KEY_G         0x22
#define KEY_H         0x23
#define KEY_J         0x24
#define KEY_K         0x25
#define KEY_L         0x26
#define KEY_SEMICOLON 0x27
#define KEY_QUOTE     0x28
#define KEY_BACKTICK  0x29
#define KEY_LSHIFT    0x2a
#define KEY_BACKSLASH 0x2b
#define KEY_Z         0x2c
#define KEY_X         0x2d
#define KEY_C         0x2e
#define KEY_V         0x2f
#define KEY_B         0x30
#define KEY_N         0x31
#define KEY_M         0x32
#define KEY_COMMA     0x33
#define KEY_PERIOD    0x34
#define KEY_SLASH     0x35
#define KEY_RSHIFT    0x36
#define KEY_ASTERISK  0x37
#define KEY_LALT      0x38
#define KEY_SPACE     0x39
#define KEY_CAPSLOCK  0x3a
#define KEY_F1        0x3b
#define KEY_F2        0x3c
#define KEY_F3        0x3d
#define KEY_F4        0x3e
#define KEY_F5        0x3f
#define KEY_F6        0x40
#define KEY_F7        0x41
#define KEY_F8        0x42
#define KEY_F9        0x43
#define KEY_F10       0x44
#define KEY_NUMLOCK   0x45
#define KEY_SCRLOCK   0x46

#define KEY_UP        0xe048
#define KEY_LEFT      0xe04b
#define KEY_RIGHT     0xe04d
#define KEY_DOWN      0xe050

struct keymapping {
    char scancode;
    char character;
    char character_shift = 0;
    char character_alt   = 0;
};

struct keymap {
    keymapping* mappings;
    int count;
};