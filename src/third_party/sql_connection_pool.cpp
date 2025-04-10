#include "sql_connection_pool.h"

ConnectionPool::ConnectionPool() 
    : m_max_conn(0)
    , m_cur_conn(0)
    , m_free_conn(0)
    , m_close_log(0) {
    // Initialize atomic members individually
    m_stats.total_connections = 0;
    m_stats.active_connections = 0;
    m_stats.connection_timeouts = 0;
    m_stats.failed_connections = 0;
}

ConnectionPool::~ConnectionPool() {
    destroy_pool();
}

ConnectionPool* ConnectionPool::get_instance() {
    static ConnectionPool conn_pool;
    return &conn_pool;
}

void ConnectionPool::init(const ConnectionPoolConfig& config) {
    try {
        m_url = config.url;
        m_port = to_string(config.port);
        m_user = config.user;
        m_password = config.password;
        m_database_name = config.database_name;
        m_close_log = config.close_log;

        for (int i = 0; i < config.max_conn; ++i) {
            MYSQL* con = nullptr;
            con = mysql_init(con);
            if (con == nullptr) {
                throw std::runtime_error("Failed to initialize MySQL connection");
            }

            con = mysql_real_connect(con, m_url.c_str(), m_user.c_str(), m_password.c_str(), 
                                   m_database_name.c_str(), config.port, nullptr, 0);
            if (con == nullptr) {
                throw std::runtime_error("Failed to connect to MySQL: " + 
                                       std::string(mysql_error(con)));
            }
            m_conn_list.emplace_back(con);
            ++m_free_conn;
            ++m_stats.total_connections;
        }
        m_reserve = locker::Semaphore(m_free_conn);
        m_max_conn = m_free_conn;
    } catch (const std::exception& e) {
        destroy_pool();
        throw;
    }
}

MYSQL* ConnectionPool::get_connection(int timeout_ms) {
    MYSQL* con = nullptr;
    if (m_conn_list.empty()) {
        return nullptr;
    }

    if (!m_reserve.wait()) {
        ++m_stats.connection_timeouts;
        return nullptr;
    }

    m_lock.lock();
    con = m_conn_list.front();
    m_conn_list.pop_front();
    --m_free_conn;
    ++m_cur_conn;
    ++m_stats.active_connections;
    m_lock.unlock();

    return con;
}

bool ConnectionPool::release_connection(MYSQL* con) {
    if (con == nullptr) {
        return false;
    }
    m_lock.lock();

    m_conn_list.push_back(con);
    ++m_free_conn;
    --m_cur_conn;
    --m_stats.active_connections;

    m_lock.unlock();

    m_reserve.post();
    return true;
}

void ConnectionPool::destroy_pool() {
    m_lock.lock();

    if (!m_conn_list.empty()) {
        for (auto it = m_conn_list.begin(); it != m_conn_list.end(); ++it) {
            MYSQL* con = *it;
            mysql_close(con);
        }
        m_cur_conn = 0;
        m_free_conn = 0;
        m_conn_list.clear();
    }

    m_lock.unlock();
}

ConnectionRAII::ConnectionRAII(MYSQL** sql, ConnectionPool* conn_pool) {
    *sql = conn_pool->get_connection();
    m_con_raii = *sql;
    m_pool_raii = conn_pool;
}

ConnectionRAII::~ConnectionRAII() {
    m_pool_raii->release_connection(m_con_raii);
}
