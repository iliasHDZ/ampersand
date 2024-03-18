#pragma once

#include <common.h>

#include "vec.hpp"

template <typename T>
class ROVec {
public:
    ROVec()
        : count(0), array(nullptr) {}

    ROVec(const T* array, usize count)
        : count(count), array(array) {}

    ROVec(const Vec<T>& vec) {
        count = vec.size();
        array = vec.begin();
    }

    usize size() const {
        return count;
    }

    const T* begin() const {
        return array;
    }

    const T* end() const {
        return array + count;
    }

    const T& get(usize index) const {
        return array[index];
    }

    inline const T& operator[](usize index) const {
        return get(index);
    }

    const inline static ROVec empty = ROVec(nullptr, 0);

private:
    usize count;
    const T* array;
};