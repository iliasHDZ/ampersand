#include "vga.hpp"

static bool vga_initialized = false;

static VGADevice instance;

static VideoTextMode text_modes[] = {
    VideoTextMode(25, 80)
};

static const u32 text_mode_count = sizeof(text_modes) / sizeof(VideoTextMode);

static const u32 video_mode_count = text_mode_count;

static VideoMode* video_modes[video_mode_count];

static ROVec<VideoMode*> video_modes_vector;

static bool video_modes_initialized = false;

static bool init_video_modes(VGADevice* device) {
    if (video_modes_initialized)
        return false;

    u32 i = 0;
    for (; i < text_mode_count; i++)
        video_modes[i] = &text_modes[i];

    for (i = 0; i < video_mode_count; i++)
        video_modes[i]->parent = device;

    video_modes_vector = ROVec<VideoMode*>(video_modes, video_mode_count);

    video_modes_initialized = true;
    return true;
}

const char* VGADevice::get_name() const {
    return "VGA Device";
}

VideoDeviceType VGADevice::get_video_type() const {
    return VideoDeviceType::VGA;
}

ROVec<VideoMode*> VGADevice::get_video_modes() const {
    return video_modes_vector;
}

const VideoMode* VGADevice::get_current_video_mode() const {
    return current_mode;
}

bool VGADevice::change_video_mode(const VideoMode* mode) {
    if (mode->parent != this)
        return false;

    return true;
}

bool VGADevice::has_text_mode() const {
    return true;
}

VideoTextDriver* VGADevice::get_text_driver() {
    return &text_driver;
}

bool VGADevice::init_device() {
    current_mode = &text_modes[0];
    text_driver.parent = this;

    if (!init_video_modes(this))
        return false;

    return true;
}

bool VGADevice::is_available() {
    return true;
}

bool VGADevice::init() {
    if (vga_initialized)
        return true;

    vga_initialized = instance.init_device();
    return vga_initialized;
}

VGADevice* VGADevice::get() {
    if (!vga_initialized)
        return nullptr;

    return &instance;
}