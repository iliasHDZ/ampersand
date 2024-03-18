#pragma once

#include <common.h>

template <typename T>
class Vec {
public:
    Vec(usize capacity)
        : count(0), capacity(capacity), array(nullptr)
    {
        array = nullptr;
        if (is_alloc_available()) {
            if (capacity > 0)
                array = new T[capacity];
            has_init = true;
        } else
            has_init = false;
    }

    Vec() : Vec(16) {}

    ~Vec() {
        if (has_init && array)
            kfree((void*)array);
    }

    void expand() {
        test_init();
        capacity += 16;
        if (array)
            array = (T*)krealloc((void*)array, sizeof(T) * capacity);
        else
            array = new T[capacity];
    }

    void append(T value) {
        test_init();
        while (count >= capacity)
            expand();

        array[count++] = value;
    }

    void remove(usize index) {
        if (count == 0)
            return;
        
        if (index >= count)
            panic("Vec: Cannot index Vec outside bounds");

        count--;
        for (usize i = index; i < count; i++)
            array[i] = array[i + 1];
    }

    usize size() const {
        return count;
    }

    T* begin() {
        test_init();
        return array;
    }

    T* end() {
        test_init();
        return array + count;
    }

    T& get(usize index) {
        test_init();
        if (index >= count)
            panic("Vec: Cannot index Vec outside bounds");

        return array[index];
    }

    inline T& operator[](usize index) {
        test_init();
        return get(index);
    }

    const inline static Vec empty = Vec(nullptr, 0);

private:
    void test_init() {
        if (!has_init) {
            if (!is_alloc_available())
                panic("Cannot use Vec with allocator not available!");

            if (capacity > 0)
                array = (T*)kmalloc( sizeof(T) * capacity );
            has_init = true;
        }
    }

private:
    usize count;
    usize capacity;
    bool has_init;
    T* array;
};