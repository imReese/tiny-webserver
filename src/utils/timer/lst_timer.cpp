#include "utils/timer/lst_timer.h"
#include "core/http/http_conn.h"

SortTimerLst::SortTimerLst() {
    m_head = nullptr;
    m_tail = nullptr;
}

SortTimerLst::~SortTimerLst() {
    UtilTimer* tmp = m_head;
    while (tmp)
    {
        m_head = tmp->next;
        delete tmp;
        tmp = m_head;
    }
}

void SortTimerLst::add_timer(UtilTimer* timer) {
    if (!timer) {
        return;
    }
    if (!m_head) {
        m_head = m_tail = timer;
        return;
    }
    if (timer->expire < m_head->expire) {
        timer->next = m_head;
        m_head->prev = timer;
        m_head = timer;
        return;
    }
    add_timer(timer, m_head);
}

void SortTimerLst::adjust_timer(UtilTimer* timer) {
    if (!timer) {
        return;
    }
    UtilTimer* tmp = timer->next;

    if (!tmp || (timer->expire < tmp->expire)) {
        return;
    }
    if (timer == m_head) {
        m_head = m_head->next;
        m_head->prev = nullptr;
        timer->next = nullptr;
        add_timer(timer, m_head);
    } else {
        timer->prev->next = timer->next;
        timer->next->prev = timer->prev;
        add_timer(timer, timer->next);
    }
}

void SortTimerLst::del_timer(UtilTimer* timer) {
    if (!timer) {
        return;
    }
    if ((timer == m_head) && (timer == m_tail)) {
        delete timer;
        m_head == nullptr;
        m_tail == nullptr;
        return;
    }
    if (timer == m_head) {
        m_head = m_head->next;
        m_head->prev = nullptr;
        delete timer;
        return;
    }
    if (timer == m_tail) {
        m_tail = m_tail->prev;
        m_tail->next == nullptr;
        delete timer;
        return;
    }
    timer->prev->next = timer->next;
    timer->next->prev = timer->prev;
    delete timer;
}

void SortTimerLst::tick() {
    if (!m_head) {
        return;
    }
    time_t cur = time(nullptr);
    UtilTimer* tmp = m_head;
    while (tmp) {
        if (cur < tmp->expire) {
            break;
        }
        tmp->cb_func(tmp->user_data);
        m_head = tmp->next;
        if (m_head) {
            m_head->prev = nullptr;
        }
        delete tmp;
        tmp = m_head;
    }
}

void SortTimerLst::add_timer(UtilTimer* timer, UtilTimer* lst_head) {
    UtilTimer* prev = lst_head;
    UtilTimer* tmp = prev->next;
    while (tmp) {
        if (timer->expire < tmp->expire) {
            prev->next = timer;
            timer->next = tmp;
            tmp->prev = timer;
            timer->prev = prev;
            break;
        }
        prev = tmp;
        tmp = tmp->next;
    }
    if (!tmp) {
        prev->next = timer;
        timer->prev = prev;
        timer->next = nullptr;
        m_tail = timer;
    }
}

Utils::Utils() : m_timeslot(0) {}

Utils::~Utils() {}

void Utils::init(int timeslot) {
    m_timeslot = timeslot;
}

int Utils::set_non_blocking(int fd) {
    int old_option = fcntl(fd, F_GETFL);
    int new_option = old_option | O_NONBLOCK;
    fcntl(fd, F_SETFL, new_option);
    return old_option;
}

void Utils::add_fd(int epollfd, int fd, bool one_shot, int trig_mode) {
    epoll_event event;
    event.data.fd = fd;

    if (trig_mode == 1) {
        event.events = EPOLLIN | EPOLLET | EPOLLRDHUP;
    } else {
        event.events = EPOLLIN | EPOLLRDHUP;
    }

    if (one_shot) {
        event.events |= EPOLLONESHOT;
    }
    epoll_ctl(epollfd, EPOLL_CTL_ADD, fd, &event);
    set_non_blocking(fd);
}

void Utils::sig_handler(int sig) {
    int save_errno = errno;
    int msg = sig;
    send(u_pipefd[1], (char*)&msg, 1, 0);
    errno = save_errno;
}

void Utils::add_sig(int sig, void(handler)(int), bool restart) {
    struct sigaction sa;
    memset(&sa, '\0', sizeof(sa));
    sa.sa_handler = handler;

    if (restart) {
        sa.sa_flags |= SA_RESTART;
    }
    sigfillset(&sa.sa_mask);
    assert(sigaction(sig, &sa, nullptr) != -1);
}

void Utils::timer_handler() {
    m_timer_lst.tick();
    alarm(m_timeslot);
}

void Utils::show_error(int connfd, const char* info) {
    send(connfd, info, strlen(info), 0);
    close(connfd);
}

int* Utils::u_pipefd = 0;
int Utils::u_epollfd = 0;

class Utils;
void cb_func(ClientData* user_data) {
    epoll_ctl(Utils::u_epollfd, EPOLL_CTL_DEL, user_data->sockfd, 0);
    assert(user_data);
    close(user_data->sockfd);
    HttpConn::m_user_count--;
}