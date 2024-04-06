#pragma once

#include "common.hpp"

class Path {
public:
    Path();
    Path(const Path& path);
    Path(const char* path);

    Path& operator=(const Path& path);

    ~Path();

    inline bool is_root() const {
        return segment_count() == 0;
    }

    inline const char* filename() const {
        return str_ptrs[count - 1];
    }

    void parse(const char* path);

    bool in(const Path& path) const;

    Path parent() const;
    
    const char* parent_as_entry_of(const Path& path);

    usize segment_count() const;

    Path resolve(const Path& path) const;

    const char*& get(usize index) const;

    const char*& operator[](usize index) const;

    inline bool is_absolute() const { return absolute; };

private:
    void dealloc_current();

private:
    usize count = 0;
    char** str_ptrs = nullptr;

    usize str_tab_size = 0;
    char* str_tab = nullptr;

    bool absolute = false;
};