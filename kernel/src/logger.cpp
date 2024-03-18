#include "logger.hpp"

TerminalOutputStream cout;

TerminalOutputStream& Log::INFO(const char* mod) {
    // cout << '\n';
    cout.set_fg(LOGGER_INFO_COLOR);
    if (mod != nullptr)
        cout << mod << ": ";
    cout.set_fg(LOGGER_COLOR);
    return cout;
}

TerminalOutputStream& Log::WARN(const char* mod) {
    // cout << '\n';
    cout.set_fg(LOGGER_WARN_COLOR);
    if (mod != nullptr)
        cout << mod << ": ";
    return cout;
}

TerminalOutputStream& Log::ERR(const char* mod) {
    // cout << '\n';
    cout.set_fg(LOGGER_ERR_COLOR);
    if (mod != nullptr)
        cout << mod << ": ";
    return cout;
}