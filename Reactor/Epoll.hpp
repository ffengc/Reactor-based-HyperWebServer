
#ifndef __EPOLL_HPP__
#define __EPOLL_HPP__

#include <sys/epoll.h>
#include <stdlib.h>

class Epoll
{
private:
    const static int __gnum = 128;
    const static int __default_timeout = 1000;
private:
    int __epfd;
    int __timeout;
public:
    Epoll(int timeout = __default_timeout) : __timeout(timeout) {}
    ~Epoll() {}
    void CreateEpoll()
    {
        __epfd = epoll_create(__gnum);
        if (__epfd < 0)
            exit(5);
    }
    bool AddSockToEpoll(int sock, uint32_t events)
    {
        struct epoll_event ev;
        ev.events = events;
        ev.data.fd = sock;
        int n = epoll_ctl(__epfd, EPOLL_CTL_ADD, sock, &ev);
        return n == 0;
    }
    int WaitEpoll(struct epoll_event revs[], int num)
    {
        // 这个函数告诉外面，现在在__epfd下，有哪些事件就绪了！有几个！
        // 所以两个参数是输出型参数！
        return epoll_wait(__epfd, revs, num, __timeout);
    }
    bool CtrlEpoll(int sock, uint32_t events)
    {
        events |= EPOLLET;
        struct epoll_event ev;
        ev.events = events;
        ev.data.fd = sock;
        int n = epoll_ctl(__epfd, EPOLL_CTL_MOD, sock, &ev);
        return n == 0;
    }
    bool DelFromEpoll(int sock)
    {
        int n = epoll_ctl(__epfd, EPOLL_CTL_DEL, sock, nullptr);
        return n == 0;
    }
};

#endif
