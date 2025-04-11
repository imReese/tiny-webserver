#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>
#include <string>
#include <memory>

#include "./config/config.h"
#include "./core/webserver.h"
#include "./utils/log/log.h"
#include "./third_party/sql_connection_pool.h"

// 全局变量
WebServer g_Server;
Config g_Config;

// 信号处理函数
void sig_handler(int sig) {
    if (sig == SIGINT || sig == SIGTERM) {
        LOG_INFO("Server shutting down...");
        exit(0);
    }
}

int main(int argc, char *argv[]) {
    // 设置默认配置
    string user = "root";
    string password = "213514";
    string databasename = "root";

    // 解析命令行参数
    g_Config.parse_args(argc, argv);

    // 初始化日志系统
    if (!Log::get_instance()->init("./ServerLog", g_Config.get_close_log(), 2000, 800000, 800)) {
        fprintf(stderr, "Failed to initialize log system\n");
        return 1;
    }

    // 设置信号处理
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = sig_handler;
    sigfillset(&sa.sa_mask);
    if (sigaction(SIGINT, &sa, NULL) == -1 || sigaction(SIGTERM, &sa, NULL) == -1) {
        LOG_ERROR("Failed to set signal handler");
        return 1;
    }

    try {
        // 初始化服务器
        g_Server.init(g_Config.get_port(), user, password, databasename, 
                   g_Config.get_log_write(), g_Config.get_opt_linger(), g_Config.get_trig_mode(),
                   g_Config.get_sql_num(), g_Config.get_thread_num(), g_Config.get_close_log(), 
                   g_Config.get_actor_model());

        // 初始化日志写入
        g_Server.init_log();
        LOG_INFO("Log system initialized");

        // 初始化数据库连接池
        g_Server.init_sql_pool();
        LOG_INFO("Database connection pool initialized");

        // 初始化线程池
        g_Server.init_thread_pool();
        LOG_INFO("Thread pool initialized");

        // 设置触发模式
        g_Server.init_trig_mode();
        LOG_INFO("Trigger mode set to %d", g_Config.get_trig_mode());

        // 开始监听
        g_Server.init_event_listen();
        LOG_INFO("Server started listening on port %d", g_Config.get_port());

        // 进入事件循环
        LOG_INFO("Entering event loop");
        g_Server.event_loop();

    } catch (const std::exception& e) {
        LOG_ERROR("Server error: %s", e.what());
        return 1;
    } catch (...) {
        LOG_ERROR("Unknown server error");
        return 1;
    }

    return 0;
}