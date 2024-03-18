#include "terminal_stream.hpp"

void TerminalOutputStream::set_terminal(TerminalOutput* target) {
    this->target = target;
}

isize TerminalOutputStream::write(const void* data, usize size) {
    if (target == nullptr) return -1;

    target->write((const char*)data, size);
    return size;
}