#pragma once

#include "arch.hpp"

enum class ExceptionType {
    GENERAL
};

class Exception;

typedef void (*ExceptionHandler)(Exception*);

class Exception {
public:
    Exception(const char* name, CPUState* state);

    inline ExceptionType get_type() const { return type; };

    inline const char* get_name() const { return name; };

    inline CPUState* get_state() { return state; };

protected:
    Exception(ExceptionType type, const char* name, CPUState* state);

private:
    ExceptionType type;
    
    const char* name;
    
    CPUState* state;

};