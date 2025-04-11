#include "webserver.h"

WebServer::WebServer() {
    m_users = new HttpConn[MAX_FD];

    char server_path[200];
    getcwd(server_path, 200);
    char root[6] = "/root";
    m_root = (char*)malloc(strlen(server_path) + strlen(root) + 1);
    strcpy(m_root, server_path);
    strcat(m_root, root);

    m_users_timer = new ClientData[MAX_FD];
}

WebServer::~WebServer() {
    close(m_epollfd);
    close(m_listenfd);
    close(m_pipefd[1]);
    close(m_pipefd[0]);
    delete[] m_users;
    delete[] m_users_timer;
    delete m_thread_pool;
}

void WebServer::init(int port, std::string user, std::string password, std::string database_name, 
                    int log_write, int opt_linger, int trig_mode, int sql_num, 
                    int thread_num, int close_log, int actor_model) {
    m_port = port;
    m_user = user;
    m_password = password;
    m_database_name = database_name;
    m_sql_num = sql_num;
    m_thread_num = thread_num;
    m_log_write = log_write;
    m_opt_linger = opt_linger;
    m_trig_mode = trig_mode;
    m_close_log = close_log;
    m_actor_model = actor_model;
}

void WebServer::init_trig_mode() {
    switch(m_trig_mode) {
        case 0: // LT + LT
            m_listen_trig_mode = 0;
            m_conn_trig_mode = 0;
            break;
        case 1: // LT + ET
            m_listen_trig_mode = 0;
            m_conn_trig_mode = 1;
            break;
        case 2: // ET + LT
            m_listen_trig_mode = 1;
            m_conn_trig_mode = 0;
            break;
        case 3: // ET + ET
            m_listen_trig_mode = 1;
            m_conn_trig_mode = 0;
            break;
        default:
            break;
    }
}

void WebServer::init_log() {
    if (m_close_log == 0) {
        if (m_log_write == 1) {
            Log::get_instance()->init("./ServerLog", m_close_log, 2000, 800000, 800);
        } else {
            Log::get_instance()->init("./ServerLog", m_close_log, 2000, 800000, 0);
        }
    }
}

void WebServer::init_sql_pool() {
    m_conn_pool = ConnectionPool::get_instance();
    ConnectionPoolConfig config{
        .url = "localhost",
        .user = m_user,
        .password = m_password,
        .database_name = m_database_name,
        .port = 3306,
        .max_conn = m_sql_num,
        .close_log = m_close_log
    };
    m_conn_pool->init(config);

    m_users->init_mysql_result(m_conn_pool);
}

void WebServer::init_thread_pool() {
    m_thread_pool = new threadpool<HttpConn>(m_actor_model, m_conn_pool, m_thread_num);
}

void WebServer::init_event_listen() {
    m_listenfd = socket(PF_INET, SOCK_STREAM, 0);
    assert(m_listenfd >= 0);

    if (m_opt_linger == 0) {
        struct linger tmp = {0, 1};
        setsockopt(m_listenfd, SOL_SOCKET, SO_LINGER, &tmp, sizeof(tmp));
    } else if (m_opt_linger == 1) {
        struct linger tmp = {1, 1};
        setsockopt(m_listenfd, SOL_SOCKET, SO_LINGER, &tmp, sizeof(tmp));
    }

    int ret = 0;
    struct sockaddr_in address;
    bzero(&address, sizeof(address));
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = htonl(INADDR_ANY);
    address.sin_port = htons(m_port);

    int flag = 1;
    setsockopt(m_listenfd, SOL_SOCKET, SO_REUSEADDR, &flag, sizeof(flag));
    ret = bind(m_listenfd, (struct sockaddr*)&address, sizeof(address));
    assert(ret >= 0);
    ret = listen(m_listenfd, 5);
    assert(ret >= 0);

    m_utils.init(TIMESLOT);

    m_epollfd = epoll_create(5);
    assert(m_epollfd != -1);

    m_utils.add_fd(m_epollfd, m_listenfd, false, m_listen_trig_mode);
    HttpConn::m_epollfd = m_epollfd;

    ret = socketpair(PF_UNIX, SOCK_STREAM, 0, m_pipefd);
    assert(ret != -1);
    m_utils.set_non_blocking(m_pipefd[1]);
    m_utils.add_fd(m_epollfd, m_pipefd[0], false, 0);

    m_utils.add_sig(SIGPIPE, SIG_IGN);
    m_utils.add_sig(SIGALRM, m_utils.sig_handler, false);
    m_utils.add_sig(SIGTERM, m_utils.sig_handler, false);

    alarm(TIMESLOT);

    Utils::u_pipefd = m_pipefd;
    Utils::u_epollfd = m_epollfd;
}

void WebServer::init_timer(int connfd, struct sockaddr_in client_address) {
    m_users[connfd].init(connfd, client_address, m_root, m_conn_trig_mode, m_close_log, m_user, m_password, m_database_name);

    m_users_timer[connfd].address = client_address;
    m_users_timer[connfd].sockfd = connfd;
    UtilTimer* timer = new UtilTimer;
    timer->user_data = &m_users_timer[connfd];
    timer->cb_func = cb_func;
    time_t cur = time(NULL);
    timer->expire = cur + 3 * TIMESLOT;
    m_users_timer[connfd].timer = timer;
    m_utils.m_timer_lst.add_timer(timer);
}

void WebServer::adjust_timer(UtilTimer* timer) {
    time_t cur = time(NULL);
    timer->expire = cur + 3 * TIMESLOT;
    m_utils.m_timer_lst.adjust_timer(timer);

    LOG_INFO("%s", "adjust time once");
}

void WebServer::handle_timer(UtilTimer* timer, int sockfd) {
    timer->cb_func(&m_users_timer[sockfd]);
    if (timer) {
        m_utils.m_timer_lst.del_timer(timer);
    }

    LOG_INFO("close fd %d", m_users_timer[sockfd].sockfd);
}

bool WebServer::handle_client_data() {
    struct sockaddr_in client_address;
    socklen_t client_addresslength = sizeof(client_address);
    if (m_listen_trig_mode == 0) {
        int connfd = accept(m_listenfd, (struct sockaddr*)&client_address, &client_addresslength);
        if (connfd < 0) {
            LOG_ERROR("%s:errno is:%d", "accept error", errno);
            return false;
        }
        if (HttpConn::m_user_count >= MAX_FD) {
            m_utils.show_error(connfd, "Internal server busy");
            LOG_ERROR("%s", "Internal server busy");
            return false;
        }
        init_timer(connfd, client_address);
    } else {
        while (1) {
            int connfd = accept(m_listenfd, (struct sockaddr*)&client_address, &client_addresslength);
            if (connfd < 0) {
                LOG_ERROR("%s:errno is:%d", "accept error", errno);
                break;
            }
            if (HttpConn::m_user_count >= MAX_FD) {
                m_utils.show_error(connfd, "Internal server busy");
                LOG_ERROR("%s", "Internal server busy");
                break;
            }
            init_timer(connfd, client_address);
        }
        return false;        
    }
    return true;
}

bool WebServer::handle_signal(bool &timeout, bool &stop_server) {
    int ret = 0;
    int sig;
    char signals[1024];
    ret = recv(m_pipefd[0], signals, sizeof(signals), 0);
    if (ret == -1) {
        return false;
    } else if (ret == 0) {
        return false;
    } else {
        for (int i = 0; i < ret; ++i) {
            switch (signals[i]) {
            case SIGALRM: 
                timeout = true;
                break;
            case SIGTERM:
                stop_server = true;
                break;
            default:
                break;
            }
        }
    }
    return true;
}

void WebServer::handle_thread(int sockfd) {
    UtilTimer* timer = m_users_timer[sockfd].timer;

    // reactor
    if(m_actor_model == 1) {
        if (timer) {
            adjust_timer(timer);
        }
        m_thread_pool->append(m_users + sockfd, 0);
        while (true) {
            if (m_users[sockfd].improv == 1) {
                if (m_users[sockfd].timer_flag == 1) {
                    handle_timer(timer, sockfd);
                    m_users[sockfd].timer_flag = 0;
                }
                m_users[sockfd].improv = 0;
                break;
            }
        }
    } else {
    // proactor 
        if (m_users[sockfd].read_once()) {
            LOG_INFO("deal with the client(%s)", inet_ntoa(m_users[sockfd].get_address()->sin_addr));
            m_thread_pool->append_p(m_users + sockfd);
            if (timer) {
                adjust_timer(timer);
            }
        } else {
            handle_timer(timer, sockfd);
        }
    }
}

void WebServer::handle_write(int sockfd) {
    UtilTimer* timer = m_users_timer[sockfd].timer;
    
    // reactor
    if (m_actor_model == 1) {
        if (timer) {
            adjust_timer(timer);
        }
        m_thread_pool->append(m_users + sockfd, 1);
        
        while (true) {
            if (m_users[sockfd].improv == 1) {
                if (m_users[sockfd].timer_flag == 1) {
                    handle_timer(timer, sockfd);
                    m_users[sockfd].timer_flag = 0;
                }
                m_users[sockfd].improv = 0;
                break;
            }
        }
    } else {
    // proactor
        if (m_users[sockfd].write()) {
            LOG_INFO("send data to the client(%s)", inet_ntoa(m_users[sockfd].get_address()->sin_addr));
            if (timer) {
                adjust_timer(timer);
            }
        } else {
            handle_timer(timer, sockfd);
        }
    }
}

void WebServer::event_loop() {
    bool timeout = false;
    bool stop_server = false;

    while (!stop_server) {
        int number = epoll_wait(m_epollfd, m_events, MAX_EVENT_NUMBER, -1);
        if (number < 0 && errno != EINTR) {
            LOG_ERROR("%s", "epoll failure");
            break;
        }

        for (int i = 0; i < number; ++i) {
            int sockfd = m_events[i].data.fd;

            if (sockfd == m_listenfd) {
                bool flag = handle_client_data();
                if (flag == false) {
                    continue;
                }
            } else if (m_events[i].events & (EPOLLRDHUP | EPOLLHUP | EPOLLERR)) {
                UtilTimer* timer = m_users_timer[sockfd].timer;
                handle_timer(timer, sockfd);
            } else if ((sockfd == m_pipefd[0]) && (m_events[i].events & EPOLLIN)) {
                bool flag = handle_signal(timeout, stop_server);
                if (flag == false) {
                    LOG_ERROR("%s", "handle_client_data failure");
                }
            } else if (m_events[i].events & EPOLLIN) {
                handle_write(sockfd);
            } else if (m_events[i].events & EPOLLOUT) {
                handle_write(sockfd);
            }
        }
        if (timeout) {
            m_utils.timer_handler();
            LOG_INFO("%s", "timer tick");
            timeout = false;
        }
    }
}