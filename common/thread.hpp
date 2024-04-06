#pragma once

#include "common.hpp"

class Mutex {
public:
    inline void lock() {
        while (locked)
            __thread_await((ThreadSignal*)this);
        locked = true;
    }

    inline void unlock() {
        locked = false;
        __thread_emit((ThreadSignal*)this);
    }

private:
    bool locked = false;
};

class MutexLock {
public:
    inline MutexLock(Mutex* mutex)
        : mutex(mutex)
    {
        mutex->lock();
    }
    
    inline MutexLock(Mutex& mutex)
        : mutex(&mutex)
    {
        mutex.lock();
    }

    inline ~MutexLock() {
        mutex->unlock();
    }

private:
    Mutex* mutex;
};