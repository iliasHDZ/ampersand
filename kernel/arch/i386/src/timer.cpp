#include "timer.hpp"
#include "int.hpp"
#include "drv/pit.hpp"

#define TIMER_CALLBACK_MAX_COUNT 8
#define TIMER_DIVISOR 1000

static u64 timer_counter = 0;

static TimerCallback timer_callbacks[TIMER_CALLBACK_MAX_COUNT];

static TimerCallback final_callback = nullptr;

u64 Timer::get_frequency() {
    return PIT_FREQUENCY / TIMER_DIVISOR;
}

void Timer::add_callback(TimerCallback callback) {
    for (u32 i = 0; i < TIMER_CALLBACK_MAX_COUNT; i++) {
        if (timer_callbacks[i] == nullptr) {
            timer_callbacks[i] = callback;
            return;
        }
    }
}

void Timer::set_final_callback(TimerCallback callback) {
    final_callback = callback;
}

u64 Timer::get_counter() {
    return timer_counter;
}

#include "io.h"

static void timer_pulse(void*) {
    for (u32 i = 0; i < TIMER_CALLBACK_MAX_COUNT; i++) {
        if (timer_callbacks[i])
            timer_callbacks[i](timer_counter);
    }

    timer_counter++;

    if (final_callback)
        final_callback(timer_counter - 1);
}

void Timer::init() {
    memset(timer_callbacks, 0, sizeof(timer_callbacks));

    timer_counter = 0;

    PIT8253::setup_channel(0, PIT_MODE_RATE_GEN, TIMER_DIVISOR);

    int_register_irq(0, timer_pulse);
}