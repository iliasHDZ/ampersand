#pragma once

#include <common.h>

#include "stream/terminal_stream.hpp"

#define LOGGER_INFO_COLOR TERM_BLUE
#define LOGGER_WARN_COLOR TERM_YELLOW
#define LOGGER_ERR_COLOR  TERM_LIGHT_RED

#define LOGGER_COLOR TERM_WHITE

namespace Log {
    TerminalOutputStream& INFO(const char* mod = nullptr);

    TerminalOutputStream& WARN(const char* mod = nullptr);

    TerminalOutputStream& ERR(const char* mod = nullptr);
};

extern TerminalOutputStream cout;