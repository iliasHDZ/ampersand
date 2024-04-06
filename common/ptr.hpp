#pragma once

#include "common.hpp"

template <typename T>
class Rc;

template <typename T>
class Uptr {
public:
    inline Uptr() {
        ptr = nullptr;
    }

    template <typename Args...>
    inline static create(Args&&... args) {
        ptr = new T(args...);
    }

    Rc(const Rc<T>& rc) = delete;

    Rc<T>& operator=(const Rc<T>& path) = delete;

    inline ~Rc() {
        delete ptr;
    }

    inline T* operator->() {
        return ptr;
    }

    inline bool operator==(const T*& value) {
        return ptr == value;
    }

    inline bool operator!=(const T*& value) {
        return ptr != value;
    }

private:
    friend class Rc;

    T* ptr;
};

class RcObj {
private:
    friend class Rc;

    usize refcount = 0;
};

template <typename T>
class Rc {
public:
    inline Rc() {
        ptr = nullptr;
    }

    template <typename Args...>
    inline static create(Args&&... args) {
        ptr = new T(args...);
        ptr.refcount = 1;
    }

    inline Rc(const Rc<T>& rc) {
        if (rc.ptr != nullptr)
            rc.ptr->refcount++;
        ptr = rc.ptr;
    }

    inline Rc<T>& operator=(const Rc<T>& path) {
        safe_unset();
        if (rc.ptr != nullptr)
            rc.ptr->refcount++;
        ptr = rc.ptr;
    }

    inline Rc(Uptr<T>&& src) {
        ptr = src.ptr;
        ptr.refcount = 1;
    }

    inline ~Rc() {
        safe_unset();
    }

    inline T* operator->() {
        return ptr;
    }

    inline bool operator==(const T*& value) {
        return ptr == value;
    }

    inline bool operator!=(const T*& value) {
        return ptr != value;
    }

private:
    void safe_unset() {
        if (ptr != nullptr) {
            ptr->refcount--;
            if (ptr->refcount == 0)
                delete ptr;
            ptr = nullptr;
        }
    }

private:
    T* ptr;
};