#pragma once

#include <common.h>
#include <logger.hpp>

class Mutex {
public:
    inline void lock() {
        while (locked)
            kthread_await((ThreadSignal*)this);
        locked = true;
    }

    inline void unlock() {
        locked = false;
        kthread_emit((ThreadSignal*)this);
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