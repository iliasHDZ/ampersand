#pragma once

#include "common.hpp"

template <typename T>
class Queue {
public:
    Queue(usize capacity)
        : count(0), capacity(capacity)
    {
        if (is_alloc_available()) {
            buf = (T*)__malloc( sizeof(T) * capacity );
            end = buf + capacity;
            rd = wr = buf;
            has_init = true;
        } else
            has_init = false;
    }

    ~Queue() {
        if (has_init)
            __free(buf);
    }

    bool can_enqueue() const {
        return count < capacity;
    }

    bool can_dequeue() const {
        return count > 0;
    }

    void enqueue(T val) {
        test_init();
        if (!can_enqueue()) return;

        *(wr++) = val;
        if (wr >= end)
            wr = buf;

        count++;
    }

    T dequeue() {
        test_init();
        
        T val = *(rd++);
        if (rd >= end)
            rd = buf;
    
        count--;
        return val;
    }

private:
    void test_init() {
        if (!has_init) {
            has_init = true;

            if (!is_alloc_available())
                panic("Cannot use Queue with allocator not available!");

            buf = (T*)__malloc( sizeof(T) * capacity );
            end = buf + capacity;
            rd = wr = buf;
        }
    }

private:
    bool has_init;

    usize count;
    usize capacity;

    T* buf;
    T* end;

    T* rd;
    T* wr;
};