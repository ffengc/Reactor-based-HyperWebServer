

#ifndef __YUFC_EPOLL__
#define __YUFC_EPOLL__

#include <iostream>
#include <sys/epoll.h>

class __epoll {
public:
    static const int gsize = 256;

public:
    static int create_epoll() {
        int epfd = epoll_create(gsize);
        if (epfd > 0)
            return epfd;
        exit(5);
    }
    static bool control_epoll(int epfd, int oper, int sock, uint32_t events) {
        struct epoll_event ev;
        ev.events = events;
        ev.data.fd = sock;
        int n = epoll_ctl(epfd, oper, sock, &ev);
        return n == 0;
    }
    static int wait_epoll(int epfd, struct epoll_event revs[], int num, int timeout) {
        return epoll_wait(epfd, revs, num, timeout);
    }
};

#endif