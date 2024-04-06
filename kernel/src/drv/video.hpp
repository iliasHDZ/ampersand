#pragma once

#include "device.hpp"

#include "../stream/terminal.hpp"

#include <common/rovec.hpp>

enum class VideoDeviceType {
    VGA
};

class VideoDevice;

struct VideoMode {
    VideoDevice* parent;
    bool is_available;
    const bool is_text_mode;

    VideoMode(bool txtmode);
};

struct VideoTextMode : VideoMode {
    u32 rows;
    u32 columns;

    VideoTextMode(u32 rows, u32 columns);
};

class VideoTextDriver;

class VideoDevice : public Device {
public:
    DeviceType get_type() const override;

    virtual VideoDeviceType get_video_type() const = 0;

    virtual ROVec<VideoMode*> get_video_modes() const = 0;

    virtual const VideoMode* get_current_video_mode() const = 0;

    virtual bool change_video_mode(const VideoMode* mode) = 0;

    virtual bool has_text_mode() const = 0;

    virtual VideoTextDriver* get_text_driver() = 0;

};

class VideoTextDriver : public TerminalOutput {
public:
    virtual VideoDevice* get_parent() const = 0;

    virtual bool is_enabled() const = 0;

    virtual u32 get_bytes_per_char() const = 0;

    virtual usize get_column_stride() const = 0;

    virtual u8* get_framebuffer() const = 0;

};