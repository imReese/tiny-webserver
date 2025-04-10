#ifndef LOCKER_H
#define LOCKER_H

#include <exception>
#include <pthread.h>
#include <semaphore.h>
#include <string>
#include <system_error>

namespace locker {

// 信号量类
class Semaphore {
private:
    sem_t m_sem;

public:
    explicit Semaphore(int value = 0) {
        if (sem_init(&m_sem, 0, value) != 0) {
            throw std::system_error(errno, std::system_category(),
                                    "Failed to initialize semaphore");
        }
    }

    ~Semaphore() {
        sem_destroy(&m_sem);
    }

    bool wait() {
        return sem_wait(&m_sem) == 0;
    }

    bool post() {
        return sem_post(&m_sem) == 0;
    }

    // 删除拷贝构造函数和赋值操作符
    Semaphore(const Semaphore&) = delete;
    Semaphore& operator=(const Semaphore&) = delete;

    // 移动构造函数和赋值操作符
    Semaphore(Semaphore&& other) noexcept {
        m_sem = other.m_sem;
        sem_init(&other.m_sem, 0, 0);  // Reset source object
    }

    Semaphore& operator=(Semaphore&& other) noexcept {
        if (this != &other) {
            sem_destroy(&m_sem);
            m_sem = other.m_sem;
            sem_init(&other.m_sem, 0, 0);  // Reset source object
        }
        return *this;
    }
};

// 互斥锁类
class Mutex {
private:
    pthread_mutex_t m_mutex;

public:
    Mutex() {
        if (pthread_mutex_init(&m_mutex, nullptr) != 0) {
            throw std::system_error(errno, std::system_category(),
                                    "Failed to initialize mutex");
        }
    }

    ~Mutex() {
        pthread_mutex_destroy(&m_mutex);
    }

    bool lock() {
        return pthread_mutex_lock(&m_mutex) == 0;
    }

    bool unlock() {
        return pthread_mutex_unlock(&m_mutex) == 0;
    }

    pthread_mutex_t* get() {
        return &m_mutex;
    }

    // 删除拷贝构造函数和赋值操作符
    Mutex(const Mutex&) = delete;
    Mutex& operator=(const Mutex&) = delete;

    // 移动构造函数和赋值操作符
    Mutex(Mutex&& other) noexcept {
        m_mutex = other.m_mutex;
        pthread_mutex_init(&other.m_mutex, nullptr);  // Reset source object
    }

    Mutex& operator=(Mutex&& other) noexcept {
        if (this != &other) {
            pthread_mutex_destroy(&m_mutex);
            m_mutex = other.m_mutex;
            pthread_mutex_init(&other.m_mutex, nullptr);  // Reset source object
        }
        return *this;
    }
};

// RAII 风格锁管理
class LockGuard {
private:
    Mutex& m_mutex;

public:
    explicit LockGuard(Mutex& mutex) : m_mutex(mutex) {
        m_mutex.lock();
    }

    ~LockGuard() {
        m_mutex.unlock();
    }

    // 删除拷贝构造函数和赋值操作符
    LockGuard(const LockGuard&) = delete;
    LockGuard& operator=(const LockGuard&) = delete;
};

// 条件变量类
class ConditionVariable {
private:
    pthread_cond_t m_cond;

public:
    ConditionVariable() {
        if (pthread_cond_init(&m_cond, nullptr) != 0) {
            throw std::system_error(errno, std::system_category(), 
                                  "Failed to initialize condition variable");
        }
    }

    ~ConditionVariable() {
        pthread_cond_destroy(&m_cond);
    }

    bool wait(Mutex& mutex) {
        return pthread_cond_wait(&m_cond, mutex.get()) == 0;
    }

    bool timed_wait(Mutex& mutex, const struct timespec* abstime) {
        return pthread_cond_timedwait(&m_cond, mutex.get(), abstime) == 0;
    }

    bool signal() {
        return pthread_cond_signal(&m_cond) == 0;
    }

    bool broadcast() {
        return pthread_cond_broadcast(&m_cond) == 0;
    }

    // Delete copy constructor and assignment operator
    ConditionVariable(const ConditionVariable&) = delete;
    ConditionVariable& operator=(const ConditionVariable&) = delete;

    // Move constructor and assignment operator
    ConditionVariable(ConditionVariable&& other) noexcept {
        m_cond = other.m_cond;
        pthread_cond_init(&other.m_cond, nullptr);  // Reset source object
    }

    ConditionVariable& operator=(ConditionVariable&& other) noexcept {
        if (this != &other) {
            pthread_cond_destroy(&m_cond);
            m_cond = other.m_cond;
            pthread_cond_init(&other.m_cond, nullptr);  // Reset source object
        }
        return *this;
    }
};

} // namespace locker

#endif