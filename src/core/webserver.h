#ifndef WEBSERVER_H
#define WEBSERVER_H

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <cassert>
#include <sys/epoll.h>

#include "../utils/threadpool/threadpool.h"
#include "./http/http_conn.h"
#include "../third_party/sql_connection_pool.h"
#include "../utils/timer/lst_timer.h"
#include "../utils/log/log.h"
#include "../utils/block_queue/block_queue.h"
#include "../utils/lock/locker.h"

const int MAX_FD = 65536;
const int MAX_EVENT_NUMBER = 10000;
const int TIMESLOT = 5;

class WebServer {
public:
    WebServer();
    ~WebServer();

    void init(int port, std::string user, std::string password, std::string database_name, 
             int log_write, int opt_linger, int trig_mode, int sql_num, 
             int thread_num, int close_log, int actor_model);

    void init_thread_pool();
    void init_sql_pool();
    void init_log();
    void init_trig_mode();
    void init_event_listen();
    void event_loop();
    void init_timer(int connfd, struct sockaddr_in client_address);
    void adjust_timer(UtilTimer *timer);
    void handle_timer(UtilTimer *timer, int sockfd);
    bool handle_client_data();
    bool handle_signal(bool& timeout, bool& stop_server);
    void handle_thread(int sockfd);
    void handle_write(int sockfd);

private:
    // 服务器配置参数
    int m_port;
    char *m_root;
    int m_log_write;
    int m_close_log;
    int m_actor_model;

    // 网络相关
    int m_pipefd[2];
    int m_epollfd;
    int m_listenfd;
    int m_opt_linger;
    int m_trig_mode;
    int m_listen_trig_mode;
    int m_conn_trig_mode;
    epoll_event m_events[MAX_EVENT_NUMBER];

    // 数据库相关
    ConnectionPool *m_conn_pool;
    std::string m_user;
    std::string m_password;
    std::string m_database_name;
    int m_sql_num;

    // 线程池相关
    threadpool<HttpConn> *m_thread_pool;
    int m_thread_num;

    // 客户端相关
    HttpConn *m_users;
    ClientData *m_users_timer;
    Utils m_utils;
};

#endif