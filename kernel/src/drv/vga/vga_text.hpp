#pragma once

#include "../video.hpp"

class VGADevice;

class VGATextDriver : public VideoTextDriver {
public:
    VideoDevice* get_parent() const override;

    bool is_enabled() const override;

    u32 get_rows() const override;

    u32 get_columns() const override;

    u32 get_bytes_per_char() const override;

    usize get_column_stride() const override;

    u8* get_framebuffer() const override;

    void fill(char ch, u8 bg, u8 fg) override;

    void set_cursor_visible(bool visible) override;

    void put_raw(char ch, u8 bg, u8 fg) override;
    
    void set_cursor_raw(u32 x, u32 y) override;

    void scroll_raw(u32 lines, u8 bg, u8 fg) override;
    
protected:
    bool guide_cursor() override;

private:
    const VideoTextMode* get_current_text_mode() const;

private:
    VGADevice* parent;
    usize index;

    friend class VGADevice;
};