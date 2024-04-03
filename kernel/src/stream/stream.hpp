#pragma once

#include <common.h>
#include <data/path.hpp>

class Stream {
    // Nothing here lol
};

struct Out {
    enum class OutType {
        hex,
        dec
    };

    OutType type;

    static inline Out hex(u8 len) {
        Out ret = { .type = OutType::hex };
        ret.hex_len  = len;
        ret.hex_pref = false;
        return ret;
    }

    static inline Out phex(u8 len) {
        Out ret = { .type = OutType::hex };
        ret.hex_len  = len;
        ret.hex_pref = true;
        return ret;
    }

    static inline Out dec() {
        return { .type = OutType::dec };
    }

    union {
        struct {
            u8   hex_len;
            bool hex_pref;
        };
    };
};

class OutputStream : public Stream {
public:
    OutputStream();

    virtual isize write(const void* data, usize size) = 0;

public:
    void write_udec(u64 val);

    void write_idec(i64 val);

    void write_uhex(u64 val, u8 len, bool pref);

    void write_ihex(i64 val, u8 len, bool pref);

    void write_uval(u64 val);

    void write_ival(i64 val);

    void write_size(u64 size);

    void hexdump(void* buf, usize size, usize origin = 0);
    
    OutputStream& operator<<(const char* val);
    
    OutputStream& operator<<(const Path& val);
    
    OutputStream& operator<<(u8 val);
    
    OutputStream& operator<<(char val);
    
    OutputStream& operator<<(u16 val);
    
    OutputStream& operator<<(i16 val);
    
    OutputStream& operator<<(u32 val);
    
    OutputStream& operator<<(i32 val);
    
    OutputStream& operator<<(u64 val);
    
    OutputStream& operator<<(i64 val);

    OutputStream& operator<<(Out manip);

private:
    Out manip;

};