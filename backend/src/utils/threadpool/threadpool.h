#ifndef THREADPOOL_H
#define THREADPOOL_H

#include <list>
#include <cstdio>
#include <exception>
#include <pthread.h>
#include <memory>

#include "../lock/locker.h"
#include "../../third_party/sql_connection_pool.h"

template <typename T>
class threadpool {
private:
    int m_thread_number;
    int m_max_requests;
    pthread_t* m_threads;
    std::list<T*> m_workqueue;
    locker::Mutex m_queuelocker;
    locker::Semaphore m_queuestat;
    ConnectionPool* m_connPool;
    int m_actor_model;

    static void* worker(void* arg);
    void run();

public:
    threadpool(int actor_model, ConnectionPool* connPool, int thread_number = 8, int max_request = 10000);
    ~threadpool();
    
    bool append(T* request, int state);
    bool append_p(T* request);
};

template <typename T>
threadpool<T>::threadpool(int actor_model, ConnectionPool* connPool, int thread_number, int max_requests) : m_actor_model(actor_model), m_thread_number(thread_number), m_max_requests(max_requests), m_threads(nullptr), m_connPool(connPool) {
    if (thread_number == 0 || max_requests <= 0) {
        throw std::exception();
    }
    m_threads = new pthread_t[m_thread_number];
    if (!m_threads) {
        throw std::exception();
    }
    for (int i = 0; i < thread_number; ++i) {
        if (pthread_create(m_threads + 1, nullptr, worker, this) != 0) {
            delete[] m_threads;
            throw std::exception();
        }
        if (pthread_detach(m_threads[i])) {
            delete[] m_threads;
            throw std::exception();
        }
    }
}

template <typename T>
threadpool<T>::~threadpool() {
    delete[] m_threads;
}

template <typename T>
bool threadpool<T>::append(T* request, int state) {
    m_queuelocker.lock();
    if (m_workqueue.size() >= m_max_requests) {
        m_queuelocker.unlock();
        return false;
    }
    request->m_state = state;
    m_workqueue.emplace_back(request);
    m_queuelocker.unlock();
    m_queuestat.post();
    return true;
}

template <typename T>
bool threadpool<T>::append_p(T* request) {
    m_queuelocker.lock();
    if (m_workqueue.size() >= m_max_requests) {
        m_queuelocker.unlock();
        return false;
    }
    m_workqueue.emplace_back(request);
    m_queuelocker.unlock();
    m_queuestat.post();
    return true;
}

template <typename T>
void* threadpool<T>::worker(void* arg) {
    threadpool* pool = (threadpool*)arg;
    pool->run();
    return pool;
}

template <typename T>
void threadpool<T>::run() {
    while (true) {
        m_queuestat.wait();
        m_queuelocker.lock();
        if (m_workqueue.empty()) {
            m_queuelocker.unlock();
            continue;
        }
        T* request = m_workqueue.front();
        m_workqueue.pop_front();
        m_queuelocker.unlock();
        if (!request) {
            continue;
        }
        if (m_actor_model == 1) {
            if (request->m_state == 0) {
                if (request->read_once()) {
                    request->improv = 1;
                    ConnectionRAII mysqlcon(&request->mysql, m_connPool);
                    request->process();
                } else {
                    request->improv = 1;
                    request->timer_flag = 1;
                }
            } else {
                if (request->write()) {
                    request->improv = 1;
                } else {
                    request->improv = 1;
                    request->timer_flag = 1;
                }
            }
        } else {
            ConnectionRAII mysqlcon(&request->mysql, m_connPool);
            request->process();
        }
    }
}

#endif