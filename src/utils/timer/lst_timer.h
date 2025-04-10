#ifndef LST_TIMER
#define LST_TIMER

#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/epoll.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <assert.h>
#include <sys/stat.h>
#include <string.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <stdarg.h>
#include <errno.h>
#include <sys/wait.h>
#include <sys/uio.h>

#include <time.h>
#include "../log/log.h"

class UtilTimer;

struct ClientData {
    sockaddr_in address;
    int sockfd;
    UtilTimer* timer;
};

class UtilTimer {
public:
    UtilTimer() : prev(NULL), next(NULL) {}

    time_t expire;

    void (*cb_func)(ClientData*);
    ClientData* user_data;
    UtilTimer* prev;
    UtilTimer* next;
};

class SortTimerLst {
private:
    void add_timer(UtilTimer* timer, UtilTimer* lst_head);
    UtilTimer* m_head;
    UtilTimer* m_tail;

public:
    SortTimerLst();
    ~SortTimerLst();

    void add_timer(UtilTimer* timer);
    void adjust_timer(UtilTimer* timer);
    void del_timer(UtilTimer* timer);
    void tick();
};

class Utils {
public:
    Utils();
    ~Utils();

    void init(int timeslot);

    int set_non_blocking(int fd);

    void add_fd(int epollfd, int fd, bool one_shoot, int trig_mode);

    static void sig_handler(int sig);

    void add_sig(int sig, void(handler)(int), bool restart = true);

    void timer_handler();

    void show_error(int connfd, const char* info);

    static int* u_pipefd;
    SortTimerLst m_timer_lst;
    static int u_epollfd;
    int m_timeslot;
};

void cb_func(ClientData* user_data);

#endif