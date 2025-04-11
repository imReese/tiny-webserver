#include "log.h"

#include <string.h>
#include <time.h>
#include <sys/time.h>
#include <stdarg.h>
#include <pthread.h>
#include <memory>
#include <stdexcept>

using namespace std;

Log::Log() 
    : m_count(0)
    , m_is_async(false)
    , m_fp(nullptr)
    , m_stop_thread(false)
    , m_thread_id(0) {
}

Log::~Log() {
    m_stop_thread = true;
    if (m_thread_id) {
        pthread_join(m_thread_id, nullptr);
    }
    if (m_fp) {
        fclose(m_fp);
    }
}

bool Log::init(const char *file_name, int close_log, int log_buf_size, int split_lines, int max_queue_size) {
    if (!file_name) {
        return false;
    }

    m_close_log = close_log;
    m_log_buf_size = log_buf_size;
    m_buf = std::make_unique<char[]>(m_log_buf_size);
    if (!m_buf) {
        return false;
    }
    memset(m_buf.get(), '\0', m_log_buf_size);
    m_split_lines = split_lines;

    time_t t = time(NULL);
    struct tm *sys_tm = localtime(&t);
    struct tm my_tm = *sys_tm;

    const char *p = strrchr(file_name, '/');
    char log_full_name[256] = {0};

    if (p == NULL) {
        snprintf(log_full_name, 255, "%d_%02d_%02d_%s", 
                my_tm.tm_year + 1900, my_tm.tm_mon + 1, my_tm.tm_mday, file_name);
    } else {
        strcpy(m_log_name, p + 1);
        strncpy(m_dir_name, file_name, p - file_name + 1);
        snprintf(log_full_name, 255, "%s%d_%02d_%02d_%s", 
                m_dir_name, my_tm.tm_year + 1900, my_tm.tm_mon + 1, my_tm.tm_mday, m_log_name);
    }

    m_today = my_tm.tm_mday;

    m_fp = fopen(log_full_name, "a");
    if (!m_fp) {
        return false;
    }

    if (max_queue_size >= 1) {
        m_is_async = true;
        m_log_queue = std::make_unique<BlockQueue<std::string>>(max_queue_size);
        if (!m_log_queue) {
            return false;
        }
        if (pthread_create(&m_thread_id, NULL, flush_log_thread, NULL) != 0) {
            m_is_async = false;
            return false;
        }
    }

    return true;
}

void Log::write_log(Level level, const char *format, ...) {
    struct timeval now = {0, 0};
    gettimeofday(&now, NULL);
    time_t t = now.tv_sec;
    struct tm *sys_tm = localtime(&t);
    struct tm my_tm = *sys_tm;
    char s[16] = {0};
    
    switch (level) {
        case Level::DEBUG:
            strcpy(s, "[debug]:");
            break;
        case Level::INFO:
            strcpy(s, "[info]:");
            break;
        case Level::WARN:
            strcpy(s, "[warn]:");
            break;
        case Level::ERROR:
            strcpy(s, "[erro]:");
            break;
    }

    m_mutex.lock();
    m_count++;
    
    if (m_today != my_tm.tm_mday || m_count % m_split_lines == 0) {
        char new_log[256] = {0};
        fflush(m_fp);
        fclose(m_fp);
        char tail[16] = {0};

        snprintf(tail, 16, "%d_%02d_%02d_", 
                my_tm.tm_year + 1900, my_tm.tm_mon + 1, my_tm.tm_mday);
        
        if (m_today != my_tm.tm_mday) {
            snprintf(new_log, 255, "%s%s%s", m_dir_name, tail, m_log_name);
            m_today = my_tm.tm_mday;
            m_count = 0;
        } else {
            snprintf(new_log, 255, "%s%s%s.%lld", 
                    m_dir_name, tail, m_log_name, m_count / m_split_lines);
        }
        m_fp = fopen(new_log, "a");
        if (!m_fp) {
            m_mutex.unlock();
            return;
        }
    }

    m_mutex.unlock();

    va_list valst;
    va_start(valst, format);

    std::string log_str;
    m_mutex.lock();

    int n = snprintf(m_buf.get(), 48, "%d-%02d-%02d %02d:%02d:%02d.%06ld %s ", 
                    my_tm.tm_year + 1900, my_tm.tm_mon + 1, my_tm.tm_mday,
                    my_tm.tm_hour, my_tm.tm_min, my_tm.tm_sec, now.tv_usec, s);
    
    int m = vsnprintf(m_buf.get() + n, m_log_buf_size - 1, format, valst);
    m_buf[n+m] = '\n';
    m_buf[n+m+1] = '\0';
    log_str = m_buf.get();

    m_mutex.unlock();

    if (m_is_async && m_log_queue && !m_log_queue->is_full()) {
        m_log_queue->push(log_str);
    } else {
        m_mutex.lock();
        fputs(log_str.c_str(), m_fp);
        m_mutex.unlock();
    }
    va_end(valst);
}

void Log::flush() {
    m_mutex.lock();
    fflush(m_fp);
    m_mutex.unlock();
}

void *Log::async_write_log() {
    std::string single_log;
    while (!m_stop_thread) {
        if (m_log_queue && m_log_queue->pop(single_log)) {
            m_mutex.lock();
            fputs(single_log.c_str(), m_fp);
            m_mutex.unlock();
        }
    }
    return nullptr;
}

void *Log::flush_log_thread(void *args) {
    Log::get_instance()->async_write_log();
    return nullptr;
}
