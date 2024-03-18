#include "../exception.hpp"

Exception::Exception(const char* name, CPUState* state)
    : Exception(ExceptionType::GENERAL, name, state) {}

Exception::Exception(ExceptionType type, const char* name, CPUState* state)
    : type(type), name(name), state(state) {}