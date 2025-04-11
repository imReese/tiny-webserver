#ifndef _CONNECTION_POOL_
#define _CONNECTION_POOL_

#include <stdio.h>
#include <list>
#include <mysql/mysql.h>
#include <error.h>
#include <string.h>
#include <iostream>
#include <string>
#include <atomic>
#include <memory>
#include <stdexcept>

#include "../utils/lock/locker.h"
#include "../utils/log/log.h"

using namespace std;

struct ConnectionPoolConfig {
    string url;
    string user;
    string password;
    string database_name;
    int port;
    int max_conn;
    int close_log;
};

class ConnectionPool {
private:
    // Connection pool state
    int m_max_conn;
    int m_cur_conn;
    int m_free_conn;
    locker::Mutex m_lock;
    list<MYSQL*> m_conn_list;
    locker::Semaphore m_reserve;

    // Connection configuration
    string m_url;
    string m_port;
    string m_user;
    string m_password;
    string m_database_name;
    int m_close_log;

    // Statistics
    struct PoolStats {
        atomic<size_t> total_connections{0};
        atomic<size_t> active_connections{0};
        atomic<size_t> connection_timeouts{0};
        atomic<size_t> failed_connections{0};

        // Default constructor
        PoolStats() = default;

        // Delete copy constructor and assignment operator
        PoolStats(const PoolStats&) = delete;
        PoolStats& operator=(const PoolStats&) = delete;
    } m_stats;

public:
    ConnectionPool();
    ~ConnectionPool();

    static ConnectionPool* get_instance();
    void init(const ConnectionPoolConfig& config);
    MYSQL* get_connection(int timeout_ms = 0);
    bool release_connection(MYSQL* conn);
    int get_free_conn() const { return m_free_conn; }
    void destroy_pool();

    const PoolStats& get_stats() const { return m_stats; }
    void reset_stats() { 
        m_stats.total_connections = 0;
        m_stats.active_connections = 0;
        m_stats.connection_timeouts = 0;
        m_stats.failed_connections = 0;
    }
};

class ConnectionRAII {
private:
    MYSQL* m_con_raii;
    ConnectionPool* m_pool_raii;

public:
    ConnectionRAII(MYSQL** con, ConnectionPool* conn_pool);
    ~ConnectionRAII();
};

#endif