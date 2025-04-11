#ifndef LOG_H
#define LOG_H

#include <stdio.h>
#include <iostream>
#include <string>
#include <stdarg.h>
#include <pthread.h>
#include <memory>
#include <atomic>

#include "../block_queue/block_queue.h"

class Log {
public:
    enum class Level {
        DEBUG = 0,
        INFO,
        WARN,
        ERROR
    };

    static Log* get_instance() {
        static Log instance;
        return &instance;
    }

    bool init(const char *file_name, int close_log, int log_buf_size = 8192, 
              int split_lines = 5000000, int max_queue_size = 0);
    void write_log(Level level, const char *format, ...);
    void flush();
    bool is_logging_enabled() const { return m_close_log == 0; }

private:
    Log();
    virtual ~Log();
    Log(const Log&) = delete;
    Log& operator=(const Log&) = delete;

    void *async_write_log();
    static void *flush_log_thread(void *args);

    char m_dir_name[128];
    char m_log_name[128];
    int m_split_lines;
    int m_log_buf_size;
    std::atomic<long long> m_count;
    int m_today;
    FILE *m_fp;
    std::unique_ptr<char[]> m_buf;
    std::unique_ptr<BlockQueue<std::string>> m_log_queue;
    bool m_is_async;
    locker::Mutex m_mutex;
    int m_close_log;
    pthread_t m_thread_id;
    bool m_stop_thread;
};

#define LOG_DEBUG(format, ...) \
    if(Log::get_instance()->is_logging_enabled()) { \
        Log::get_instance()->write_log(Log::Level::DEBUG, format, ##__VA_ARGS__); \
        Log::get_instance()->flush(); \
    }

#define LOG_INFO(format, ...) \
    if(Log::get_instance()->is_logging_enabled()) { \
        Log::get_instance()->write_log(Log::Level::INFO, format, ##__VA_ARGS__); \
        Log::get_instance()->flush(); \
    }

#define LOG_WARN(format, ...) \
    if(Log::get_instance()->is_logging_enabled()) { \
        Log::get_instance()->write_log(Log::Level::WARN, format, ##__VA_ARGS__); \
        Log::get_instance()->flush(); \
    }

#define LOG_ERROR(format, ...) \
    if(Log::get_instance()->is_logging_enabled()) { \
        Log::get_instance()->write_log(Log::Level::ERROR, format, ##__VA_ARGS__); \
        Log::get_instance()->flush(); \
    }

#endif