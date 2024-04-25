

#ifndef __YUFC_EPOLL__
#define __YUFC_EPOLL__

#include <iostream>
#include <sys/epoll.h>

class __epoll {
private:
    int __epoll_fd;
    int __timeout;

public:
    const static int gnum = 128;

public:
    __epoll(int timeout)
        : __timeout(timeout) { }
    ~__epoll() { }

public:
    void create_poll() {
        __epoll_fd = epoll_create(gnum);
        if (__epoll_fd < 0)
            exit(5);
    }
    bool add_sock_to_poll(int sock, uint32_t events) {
        struct epoll_event ev;
        ev.events = events;
        ev.data.fd = sock;
        int n = epoll_ctl(__epoll_fd, EPOLL_CTL_ADD, sock, &ev);
        return n == 0;
    }
    int wait_poll(struct epoll_event revs[], int num) {
        return epoll_wait(__epoll_fd, revs, num, __timeout);
    }

public:
};

#endif