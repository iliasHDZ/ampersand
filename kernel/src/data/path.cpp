#include "path.hpp"

Path::Path() {}

Path::Path(const Path& path) {
    (*this) = path;
}

Path::Path(const char* path) {
    parse(path);
}

Path& Path::operator=(const Path& path) {
    dealloc_current();

    str_ptrs = new char*[path.count];
    str_tab  = new char[path.str_tab_size];
    
    memcpy(str_ptrs, path.str_ptrs, sizeof(char*) * path.count);
    memcpy(str_tab,  path.str_tab,  path.str_tab_size);

    count = path.count;
    str_tab_size = path.str_tab_size;
    return (*this);
}

Path::~Path() {
    dealloc_current();
}

void Path::parse(const char* path) {
    dealloc_current();

    count = 0;

    if (!*path)
        return;

    absolute = (*path == '/');

    usize len = strlen(path);
    str_tab = new char[len + 1];
    memcpy(str_tab, path, len);
    str_tab[len] = 0;

    str_tab_size = len + 1;

    bool prev_slash = false;
    for (usize i = 0; i < len; i++) {
        if (prev_slash)
            count++;
        prev_slash = str_tab[i] == '/';
    }

    if (count > 0)
        str_ptrs = new char*[count];
    else
        return;

    usize str_ptr_counter = 0;

    prev_slash = false;
    for (usize i = 0; i < len; i++) {
        if (prev_slash) {
            str_ptrs[str_ptr_counter++] = str_tab + i;
        
        }

        prev_slash = str_tab[i] == '/';
        if (prev_slash)
            str_tab[i] = 0;
    }
}

bool Path::in(const Path& path) const {
    if (count > path.count)
        return false;

    for (usize i = 0; i < path.count; i++) {
        if (!streq((*this)[i], path[i]))
            return false;
    }

    return true;
}

Path Path::parent() const {
    Path ret;
    if (count <= 1)
        return ret;

    char* end = str_ptrs[count - 1];
    ret.str_tab_size = end - str_tab;
    ret.count = count - 1;

    ret.str_ptrs = new char*[ret.count];
    ret.str_tab  = new char[ret.str_tab_size];
    
    memcpy(ret.str_ptrs, str_ptrs, sizeof(char*) * ret.count);
    memcpy(ret.str_tab,  str_tab,  ret.str_tab_size);

    return ret;
}

const char* Path::parent_as_entry_of(const Path& path) {
    if (count >= path.count)
        return nullptr;

    return path[count];
}

usize Path::segment_count() const {
    return count;
}

Path Path::resolve(const Path& path) const {
    if (path.absolute)
        return path;
    
    Path ret;

    ret.count = count + path.count;
    ret.str_tab_size = str_tab_size + path.count;
    ret.str_ptrs = new char*[ret.count];
    ret.str_tab  = new char[ret.str_tab_size];

    safe_copy(ret.str_ptrs, str_ptrs, count);
    safe_copy(ret.str_ptrs + count, path.str_ptrs, path.count);

    safe_copy(ret.str_tab, str_tab, str_tab_size);
    safe_copy(ret.str_tab + str_tab_size, path.str_tab, path.str_tab_size);

    return ret;
}

static const char* undefined = nullptr;

const char*& Path::get(usize index) const {
    if (index >= count) {
        panic("Path: Attempting to index a path segment beyond array bounds");
        return undefined;
    }

    return (const char*&)str_ptrs[index];
}

const char*& Path::operator[](usize index) const {
    return get(index);
}

void Path::dealloc_current() {
    if (str_ptrs != nullptr)
        delete str_ptrs;
    str_ptrs = nullptr;

    if (str_tab != nullptr)
        delete str_tab;
    str_tab = nullptr;
}