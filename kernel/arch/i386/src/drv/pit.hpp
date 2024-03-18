#pragma once

#include <common.h>

#define PIT_MODE_INT_TERM_COUNT  0
#define PIT_MODE_HWRT_ONE_SHOT   1
#define PIT_MODE_RATE_GEN        2
#define PIT_MODE_SQUARE_WAVE_GEN 3
#define PIT_MODE_SW_TRIG_STROBE  4
#define PIT_MODE_HW_TRIG_STROBE  5

#define PIT_FREQUENCY 1193180

#define PIT_CHANNEL0_PORT 0x40
#define PIT_CHANNEL1_PORT 0x41
#define PIT_CHANNEL2_PORT 0x42
#define PIT_COMMAND_PORT  0x43

class PIT8253 {
public:
    static void setup_channel(u8 ch, u8 mode, u16 divisor);
};