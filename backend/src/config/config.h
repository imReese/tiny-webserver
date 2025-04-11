#ifndef CONFIG_H
#define CONFIG_H

#include <string>
#include <stdexcept>
#include <limits>

class Config {
public:
    // 构造函数和析构函数
    Config();
    ~Config() = default;

    // 禁用拷贝和移动
    Config(const Config&) = delete;
    Config& operator=(const Config&) = delete;
    Config(Config&&) = delete;
    Config& operator=(Config&&) = delete;

    // 配置解析
    bool parse_args(int argc, char* argv[]);
    bool load_from_file(const std::string& filename);
    bool save_to_file(const std::string& filename);

    // 配置验证
    bool validate() const;
    std::string get_error_message() const { return m_error_message; }

    // 配置参数访问器
    int get_port() const { return m_port; }
    int get_log_write() const { return m_log_write; }
    int get_trig_mode() const { return m_trig_mode; }
    int get_listen_trig_mode() const { return m_listen_trig_mode; }
    int get_conn_trig_mode() const { return m_conn_trig_mode; }
    int get_opt_linger() const { return m_opt_linger; }
    int get_sql_num() const { return m_sql_num; }
    int get_thread_num() const { return m_thread_num; }
    int get_close_log() const { return m_close_log; }
    int get_actor_model() const { return m_actor_model; }

    // 配置参数设置器
    void set_port(int port);
    void set_log_write(int log_write);
    void set_trig_mode(int trig_mode);
    void set_opt_linger(int opt_linger);
    void set_sql_num(int sql_num);
    void set_thread_num(int thread_num);
    void set_close_log(int close_log);
    void set_actor_model(int actor_model);

private:
    // 配置参数
    int m_port;
    int m_log_write;
    int m_trig_mode;
    int m_listen_trig_mode;
    int m_conn_trig_mode;
    int m_opt_linger;
    int m_sql_num;
    int m_thread_num;
    int m_close_log;
    int m_actor_model;

    // 错误信息
    std::string m_error_message = "";

    // 参数验证
    bool validate_port(int port) const;
    bool validate_log_write(int log_write) const;
    bool validate_trig_mode(int trig_mode) const;
    bool validate_opt_linger(int opt_linger) const;
    bool validate_sql_num(int sql_num) const;
    bool validate_thread_num(int thread_num) const;
    bool validate_close_log(int close_log) const;
    bool validate_actor_model(int actor_model) const;

    // 默认值
    static constexpr int DEFAULT_PORT = 9000;
    static constexpr int DEFAULT_LOG_WRITE = 0;
    static constexpr int DEFAULT_TRIG_MODE = 0;
    static constexpr int DEFAULT_OPT_LINGER = 0;
    static constexpr int DEFAULT_SQL_NUM = 8;
    static constexpr int DEFAULT_THREAD_NUM = 8;
    static constexpr int DEFAULT_CLOSE_LOG = 0;
    static constexpr int DEFAULT_ACTOR_MODEL = 0;

    // 参数范围
    static constexpr int MIN_PORT = 1024;
    static constexpr int MAX_PORT = 65535;
    static constexpr int MIN_SQL_NUM = 1;
    static constexpr int MAX_SQL_NUM = 100;
    static constexpr int MIN_THREAD_NUM = 1;
    static constexpr int MAX_THREAD_NUM = 100;
};

#endif