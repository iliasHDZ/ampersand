#include "stream.hpp"

static char val_write_buffer[32];

static constexpr char* val_write_buffer_end = val_write_buffer + sizeof(val_write_buffer);

OutputStream::OutputStream()
    : manip(Out::dec()) {}

void OutputStream::write_udec(u64 val) {
    if (val == 0) {
        *this << '0';
        return;
    }

    char* ptr = val_write_buffer_end;

    while (val) {
        *(--ptr) = '0' + (val % 10);
        val /= 10;
    }

    write(ptr, val_write_buffer_end - ptr);
}

void OutputStream::write_idec(i64 val) {
    if (val < 0) {
        *this << '-';
        val = -val;
    }

    write_udec(val);
}
    
void OutputStream::write_uhex(u64 val, u8 len, bool pref) {
    if (pref)
        *this << "0x";

    len = min(len, (u8)32);

    if (len) {
        u16 bitOff = len * 4;
        char* ptr = val_write_buffer;

        while (bitOff) {
            bitOff -= 4;
            *(ptr++) = get_hex_char((val >> bitOff) & 0xf);
        }

        write(val_write_buffer, len);
    } else {
        if (val == 0) {
            *this << '0';
            return;
        }

        char* ptr = val_write_buffer_end;

        while (val) {
            *(--ptr) = get_hex_char(val & 0xf);
            val >>= 4;
        }

        write(ptr, val_write_buffer_end - ptr);
    }
}

void OutputStream::write_ihex(i64 val, u8 len, bool pref) {
    if (val < 0) {
        *this << '-';
        val = -val;
    }

    write_uhex(val, len, pref);
}

void OutputStream::write_uval(u64 val) {
    switch (manip.type) {
    case Out::OutType::hex:
        write_uhex(val, manip.hex_len, manip.hex_pref);
        break;
    case Out::OutType::dec:
        write_udec(val);
        break;
    }
}

void OutputStream::write_ival(i64 val) {
    switch (manip.type) {
    case Out::OutType::hex:
        write_ihex(val, manip.hex_len, manip.hex_pref);
        break;
    case Out::OutType::dec:
        write_idec(val);
        break;
    }
}

static const char* size_units[] = {
    "b", "KiB", "MiB", "GiB", "TiB"
};

void OutputStream::write_size(u64 size) {
    u64 frac = 0;

    u32 uidx = 0;

    while (size >= 1024) {
        frac = ((size & 0x3ff) << 54) | (frac >> 10);
        size >>= 10;
        uidx++;
    }

    if (uidx >= (sizeof(size_units) / sizeof(const char*)))
        uidx = (sizeof(size_units) / sizeof(const char*)) - 1;

    write_udec(size);
    *this << '.';

    frac >>= 32;

    for (int i = 0; i < 2; i++) {
        frac *= 10;
        *this << (char)('0' + (char)(frac >> 32));
        frac &= 0xffffffff;
    }

    *this << ' ' << size_units[uidx];
}

void OutputStream::hexdump(void* buffer, usize size, usize origin) {
    u8 column = 0;

    *this << '\n';

    u8* buf = (u8*)buffer;

    while (size--) {
        if (column == 0) {
            write_uhex(origin, 8, false);
            *this << ": ";
        }

        write_uhex(*(buf++), 2, false);

        if (column == 15) {
            *this << '\n';
            column = 0;
        } else {
            *this << ' ';
            column++;
        }

        origin++;
    }
}

OutputStream& OutputStream::operator<<(const char* val) {
    write(val, strlen(val));
    return *this;
}
    
OutputStream& OutputStream::operator<<(const Path& val) {
    if (val.is_absolute())
        (*this) << '/';
    
    for (isize i = 0; i < val.segment_count(); i++) {
        (*this) << val[i];
        if (i < val.segment_count() - 1)
            (*this) << '/';
    }

    return *this;
}

OutputStream& OutputStream::operator<<(u8 val) {
    write_uval(val);
    return *this;
}

OutputStream& OutputStream::operator<<(char val) {
    write(&val, 1);
    return *this;
}

OutputStream& OutputStream::operator<<(u16 val) {
    write_uval(val);
    return *this;
}

OutputStream& OutputStream::operator<<(i16 val) {
    write_ival(val);
    return *this;
}

OutputStream& OutputStream::operator<<(u32 val) {
    write_uval(val);
    return *this;
}

OutputStream& OutputStream::operator<<(i32 val) {
    write_ival(val);
    return *this;
}

OutputStream& OutputStream::operator<<(u64 val) {
    write_uval(val);
    return *this;
}

OutputStream& OutputStream::operator<<(i64 val) {
    write_ival(val);
    return *this;
}

OutputStream& OutputStream::operator<<(Out manip) {
    this->manip = manip;
    return *this;
}