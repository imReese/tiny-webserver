#ifndef BLOCK_QUEUE_H
#define BLOCK_QUEUE_H

#include <iostream>
#include <stdlib.h>
#include <pthread.h>
#include <sys/time.h>
#include <memory>
#include <stdexcept>
#include <vector>

#include "../utils/lock/locker.h"

using namespace std;

template <class T>
class BlockQueue {
private:
    locker::Mutex m_mutex;
    locker::ConditionVariable m_cond;
    int m_max_size;
    int m_size;
    std::unique_ptr<T[]> m_array;
    int m_front;
    int m_back;

public:
    BlockQueue(int max_size = 1000) {
        if (max_size <= 0) {
            throw std::invalid_argument("Queue size must be positive");
        }

        m_max_size = max_size;
        m_array = std::make_unique<T[]>(max_size);
        if (!m_array) {
            throw std::runtime_error("Failed to allocate memory for queue");
        }
        m_size = 0;
        m_front = -1;
        m_back = -1;
    }

    // Move constructor
    BlockQueue(BlockQueue&& other) noexcept 
        : m_mutex(std::move(other.m_mutex))
        , m_cond(std::move(other.m_cond))
        , m_max_size(other.m_max_size)
        , m_size(other.m_size)
        , m_array(std::move(other.m_array))
        , m_front(other.m_front)
        , m_back(other.m_back) {
    }

    // Move assignment operator
    BlockQueue& operator=(BlockQueue&& other) noexcept {
        if (this != &other) {
            m_mutex = std::move(other.m_mutex);
            m_cond = std::move(other.m_cond);
            m_max_size = other.m_max_size;
            m_size = other.m_size;
            m_array = std::move(other.m_array);
            m_front = other.m_front;
            m_back = other.m_back;
        }
        return *this;
    }

    // Delete copy constructor and assignment operator
    BlockQueue(const BlockQueue&) = delete;
    BlockQueue& operator=(const BlockQueue&) = delete;

    void clear() {
        m_mutex.lock();
        m_array.reset();
        m_size = 0;
        m_front = -1;
        m_back = -1;
        m_mutex.unlock();
    }

    bool is_full() {
        m_mutex.lock();
        bool result = (m_size >= m_max_size);
        m_mutex.unlock();
        return result;
    }

    bool is_empty() {
        m_mutex.lock();
        bool result = (m_size == 0);
        m_mutex.unlock();
        return result;
    }

    bool front(T &value) {
        m_mutex.lock();
        if (m_size == 0) {
            m_mutex.unlock();
            return false;
        }
        value = m_array[m_front];
        m_mutex.unlock();
        return true;
    }

    bool back(T &value) {
        m_mutex.lock();
        if (m_size == 0) {
            m_mutex.unlock();
            return false;
        }
        value = m_array[m_back];
        m_mutex.unlock();
        return true;
    }

    int size() {
        m_mutex.lock();
        int tmp = m_size;
        m_mutex.unlock();
        return tmp;
    }

    int max_size() const {
        return m_max_size;
    }

    // Batch add elements
    bool push_batch(const std::vector<T>& items) {
        m_mutex.lock();
        if (m_size + items.size() > m_max_size) {
            m_mutex.unlock();
            return false;
        }
        for (const auto& item : items) {
            m_back = (m_back + 1) % m_max_size;
            m_array[m_back] = item;
            m_size++;
        }
        m_cond.broadcast();
        m_mutex.unlock();
        return true;
    }

    bool push(const T &item) {
        m_mutex.lock();
        if (m_size >= m_max_size) {
            m_cond.broadcast();
            m_mutex.unlock();
            return false;
        }
        m_back = (m_back + 1) % m_max_size;
        m_array[m_back] = item;
        m_size++;
        m_cond.broadcast();
        m_mutex.unlock();
        return true;
    }

    // Batch get elements
    bool pop_batch(std::vector<T>& items, int count) {
        m_mutex.lock();
        while (m_size < count) {
            if (!m_cond.wait(m_mutex)) {
                m_mutex.unlock();
                return false;
            }
        }
        items.clear();
        items.reserve(count);
        for (int i = 0; i < count; ++i) {
            m_front = (m_front + 1) % m_max_size;
            items.push_back(m_array[m_front]);
            m_size--;
        }
        m_mutex.unlock();
        return true;
    }

    bool pop(T &item) {
        m_mutex.lock();
        while (m_size <= 0) {
            if (!m_cond.wait(m_mutex)) {
                m_mutex.unlock();
                return false;
            }
        }
        m_front = (m_front + 1) % m_max_size;
        item = m_array[m_front];
        m_size--;
        m_mutex.unlock();
        return true;
    }

    bool pop(T &item, int ms_timeout) {
        struct timespec t = {0, 0};
        struct timeval now = {0, 0};
        gettimeofday(&now, NULL);
        m_mutex.lock();
        if (m_size <= 0) {
            t.tv_sec = now.tv_sec + ms_timeout / 1000;
            t.tv_nsec = (ms_timeout % 1000) * 1000;
            if (!m_cond.timed_wait(m_mutex, &t)) {
                m_mutex.unlock();
                return false;
            }
        }
        if (m_size <= 0) {
            m_mutex.unlock();
            return false;
        }
        m_front = (m_front + 1) % m_max_size;
        item = m_array[m_front];
        m_size--;
        m_mutex.unlock();
        return true;
    }
};

#endif