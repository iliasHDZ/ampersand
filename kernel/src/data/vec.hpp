#pragma once

#include <common.h>

#define VEC_NOT_FOUND 0xffffffff

template <typename T>
class Vec {
public:
    Vec(usize capacity)
        : count(0), capacity(capacity), array(nullptr)
    {
        array = nullptr;
        if (is_alloc_available()) {
            if (capacity > 0)
                alloc_array();
            has_init = true;
        } else
            has_init = false;
    }

    Vec() : Vec(16) {}

    ~Vec() {
        if (has_init && array)
            dealloc_array();
    }

    Vec(const Vec& vec) {
        test_init();
    
        resize_array(vec.count);
        safe_copy(array, vec.array, count);
    }

    Vec& operator=(const Vec& vec) {
        test_init();
        if (has_init && array)
            dealloc_array();
    
        resize_array(vec.count);
        safe_copy(array, vec.array, count);
        return *this;
    }

    void append(T value) {
        test_init();
        
        resize_array(count + 1);
        array[count - 1] = value;
    }

    void remove(usize index) {
        if (count == 0)
            return;
        
        if (index >= count)
            panic("Vec: Cannot index Vec outside bounds");

        for (usize i = index + 1; i < count; i++)
            array[i - 1] = array[i];

        resize_array(count - 1);
    }

    usize index_of(T val) {
        for (usize i = 0; i < count; i++) {
            if (array[i] == val)
                return i;
        }

        return VEC_NOT_FOUND;
    }

    bool has(T val) {
        for (auto& v : (*this)) {
            if (v == val)
                return true;
        }

        return false;
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
    void expand() {
        test_init();
        capacity += 16;
        if (array)
            array = (T*)krealloc((void*)array, sizeof(T) * capacity);
        else
            alloc_array();
    }

    void test_init() {
        if (!has_init) {
            if (!is_alloc_available())
                panic("Cannot use Vec with allocator not available!");

            if (capacity > 0)
                alloc_array();
            has_init = true;
        }
    }

    void alloc_array() {
        if (array != nullptr)
            return;

        array = (T*)kmalloc( sizeof(T) * capacity );
        for (usize i = 0; i < count; i++)
            array[i] = T();
    }

    void dealloc_array() {
        if (array == nullptr)
            return;

        for (usize i = 0; i < count; i++)
            array[i].~T();

        kfree(array);
        array = nullptr;
    }

    void resize_array(usize new_count) {
        if (new_count < count) {
            for (usize i = new_count; i < count; i++)
                array[i].~T();
        }

        if (new_count > capacity)
            expand();

        if (count > new_count) {
            for (usize i = count; i < new_count; i++)
                array[i] = T();
        }
    }

private:
    usize count = 0;
    usize capacity = 10;
    bool has_init;
    T* array;
};