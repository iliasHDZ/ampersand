#pragma once

#include <common.h>

typedef void(*KeyboardTransmitCallback)(u8);

void arch_set_keyboard_callback(KeyboardTransmitCallback cb);