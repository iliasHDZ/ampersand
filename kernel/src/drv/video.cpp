#include "video.hpp"

VideoMode::VideoMode(bool txtmode) : parent(nullptr), is_available(true), is_text_mode(txtmode) {}

VideoTextMode::VideoTextMode(u32 rows, u32 columns): VideoMode(true), rows(rows), columns(columns) {}

DeviceType VideoDevice::get_type() const {
    return DeviceType::VIDEO;
}