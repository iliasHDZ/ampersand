#pragma once

#include <common.h>

enum class DeviceType {
    VIDEO
};

class Device {
public:
    virtual const char* get_name() const = 0;

    virtual DeviceType get_type() const = 0;

};