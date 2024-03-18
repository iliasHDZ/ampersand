#pragma once

#include <common.h>

typedef void(*TimerCallback)(u64);

class Timer {
public:
    static u64 get_frequency();

    static void add_callback(TimerCallback callback);

    static void set_final_callback(TimerCallback callback);

    static u64 get_counter();

    static void init();
};