#pragma once

#include "../video.hpp"

#include "vga_text.hpp"

class VGADevice : public VideoDevice {
public:
    const char* get_name() const override;

    VideoDeviceType get_video_type() const override;

    ROVec<VideoMode*> get_video_modes() const override;

    const VideoMode* get_current_video_mode() const override;

    bool change_video_mode(const VideoMode* mode) override;

    bool has_text_mode() const override;

    VideoTextDriver* get_text_driver() override;

private:
    bool init_device();

public:
    static bool is_available();

    static bool init();

    static VGADevice* get();

private:
    const VideoMode* current_mode;

    VGATextDriver text_driver;

};