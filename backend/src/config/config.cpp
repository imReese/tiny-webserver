#include "config.h"
#include <fstream>
#include <sstream>
#include <getopt.h>
#include <json/json.h>

Config::Config() {
    // 设置默认值
    m_port = DEFAULT_PORT;
    m_log_write = DEFAULT_LOG_WRITE;
    m_trig_mode = DEFAULT_TRIG_MODE;
    m_listen_trig_mode = 0;
    m_conn_trig_mode = 0;
    m_opt_linger = DEFAULT_OPT_LINGER;
    m_sql_num = DEFAULT_SQL_NUM;
    m_thread_num = DEFAULT_THREAD_NUM;
    m_close_log = DEFAULT_CLOSE_LOG;
    m_actor_model = DEFAULT_ACTOR_MODEL;
}

bool Config::parse_args(int argc, char* argv[]) {
    int opt;
    const char* str = "p:l:m:o:s:t:c:a:";
    while ((opt = getopt(argc, argv, str)) != -1) {
        switch (opt) {
            case 'p': {
                int port = atoi(optarg);
                if (!validate_port(port)) {
                    m_error_message = "Invalid port number";
                    return false;
                }
                m_port = port;
                break;
            }
            case 'l': {
                int log_write = atoi(optarg);
                if (!validate_log_write(log_write)) {
                    m_error_message = "Invalid log write mode";
                    return false;
                }
                m_log_write = log_write;
                break;
            }
            case 'm': {
                int trig_mode = atoi(optarg);
                if (!validate_trig_mode(trig_mode)) {
                    m_error_message = "Invalid trigger mode";
                    return false;
                }
                m_trig_mode = trig_mode;
                break;
            }
            case 'o': {
                int opt_linger = atoi(optarg);
                if (!validate_opt_linger(opt_linger)) {
                    m_error_message = "Invalid linger option";
                    return false;
                }
                m_opt_linger = opt_linger;
                break;
            }
            case 's': {
                int num = atoi(optarg);
                if (!validate_sql_num(num)) {
                    m_error_message = "Invalid SQL connection number";
                    return false;
                }
                m_sql_num = num;
                break;
            }
            case 't': {
                int num = atoi(optarg);
                if (!validate_thread_num(num)) {
                    m_error_message = "Invalid thread number";
                    return false;
                }
                m_thread_num = num;
                break;
            }
            case 'c': {
                int close = atoi(optarg);
                if (!validate_close_log(close)) {
                    m_error_message = "Invalid close log option";
                    return false;
                }
                m_close_log = close;
                break;
            }
            case 'a': {
                int model = atoi(optarg);
                if (!validate_actor_model(model)) {
                    m_error_message = "Invalid actor model";
                    return false;
                }
                m_actor_model = model;
                break;
            }
            default:
                m_error_message = "Unknown option";
                return false;
        }
    }
    return validate();
}

bool Config::load_from_file(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        m_error_message = "Failed to open config file";
        return false;
    }

    Json::Value root;
    Json::Reader reader;
    if (!reader.parse(file, root)) {
        m_error_message = "Failed to parse config file";
        return false;
    }

    try {
        set_port(root.get("port", DEFAULT_PORT).asInt());
        set_log_write(root.get("log_write", DEFAULT_LOG_WRITE).asInt());
        set_trig_mode(root.get("trig_mode", DEFAULT_TRIG_MODE).asInt());
        set_opt_linger(root.get("opt_linger", DEFAULT_OPT_LINGER).asInt());
        set_sql_num(root.get("sql_num", DEFAULT_SQL_NUM).asInt());
        set_thread_num(root.get("thread_num", DEFAULT_THREAD_NUM).asInt());
        set_close_log(root.get("close_log", DEFAULT_CLOSE_LOG).asInt());
        set_actor_model(root.get("actor_model", DEFAULT_ACTOR_MODEL).asInt());
    } catch (const std::exception& e) {
        m_error_message = std::string("Error loading config: ") + e.what();
        return false;
    }

    return validate();
}

bool Config::save_to_file(const std::string& filename) {
    Json::Value root;
    root["port"] = m_port;
    root["log_write"] = m_log_write;
    root["trig_mode"] = m_trig_mode;
    root["opt_linger"] = m_opt_linger;
    root["sql_num"] = m_sql_num;
    root["thread_num"] = m_thread_num;
    root["close_log"] = m_close_log;
    root["actor_model"] = m_actor_model;

    std::ofstream file(filename);
    if (!file.is_open()) {
        m_error_message = "Failed to open config file for writing";
        return false;
    }

    Json::StyledWriter writer;
    file << writer.write(root);
    return true;
}

bool Config::validate() const {
    return validate_port(m_port) &&
           validate_log_write(m_log_write) &&
           validate_trig_mode(m_trig_mode) &&
           validate_opt_linger(m_opt_linger) &&
           validate_sql_num(m_sql_num) &&
           validate_thread_num(m_thread_num) &&
           validate_close_log(m_close_log) &&
           validate_actor_model(m_actor_model);
}

// 参数验证函数
bool Config::validate_port(int port) const {
    return port >= MIN_PORT && port <= MAX_PORT;
}

bool Config::validate_log_write(int log_write) const {
    return log_write == 0 || log_write == 1;
}

bool Config::validate_trig_mode(int trig_mode) const {
    return trig_mode >= 0 && trig_mode <= 3;
}

bool Config::validate_opt_linger(int opt_linger) const {
    return opt_linger == 0 || opt_linger == 1;
}

bool Config::validate_sql_num(int sql_num) const {
    return sql_num >= MIN_SQL_NUM && sql_num <= MAX_SQL_NUM;
}

bool Config::validate_thread_num(int thread_num) const {
    return thread_num >= MIN_THREAD_NUM && thread_num <= MAX_THREAD_NUM;
}

bool Config::validate_close_log(int close_log) const {
    return close_log == 0 || close_log == 1;
}

bool Config::validate_actor_model(int actor_model) const {
    return actor_model == 0 || actor_model == 1;
}

// 设置器函数
void Config::set_port(int port) {
    if (validate_port(port)) {
        m_port = port;
    } else {
        throw std::invalid_argument("Invalid port number");
    }
}

void Config::set_log_write(int log_write) {
    if (validate_log_write(log_write)) {
        m_log_write = log_write;
    } else {
        throw std::invalid_argument("Invalid log write mode");
    }
}

void Config::set_trig_mode(int trig_mode) {
    if (validate_trig_mode(trig_mode)) {
        m_trig_mode = trig_mode;
    } else {
        throw std::invalid_argument("Invalid trigger mode");
    }
}

void Config::set_opt_linger(int opt_linger) {
    if (validate_opt_linger(opt_linger)) {
        m_opt_linger = opt_linger;
    } else {
        throw std::invalid_argument("Invalid linger option");
    }
}

void Config::set_sql_num(int num) {
    if (validate_sql_num(num)) {
        m_sql_num = num;
    } else {
        throw std::invalid_argument("Invalid SQL connection number");
    }
}

void Config::set_thread_num(int num) {
    if (validate_thread_num(num)) {
        m_thread_num = num;
    } else {
        throw std::invalid_argument("Invalid thread number");
    }
}

void Config::set_close_log(int close) {
    if (validate_close_log(close)) {
        m_close_log = close;
    } else {
        throw std::invalid_argument("Invalid close log option");
    }
}

void Config::set_actor_model(int model) {
    if (validate_actor_model(model)) {
        m_actor_model = model;
    } else {
        throw std::invalid_argument("Invalid actor model");
    }
}