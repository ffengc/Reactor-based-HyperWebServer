

#ifndef __YUFC_EPOLL_SERVER__
#define __YUFC_EPOLL_SERVER__

#include "epoll.hpp"
#include "log.hpp"
#include "sock.hpp"
#include <assert.h>
#include <functional>
#include <iostream>
#include <string>

namespace ns_epoll {
static const int default_port = 8080;
static const int g_revs_num = 64; // 根据业务可以调整
class epoll_server {
private:
    int __listen_sock; // 监听套接字
    int __epoll_fd; // epoll模型fd
    uint16_t __port; // 服务器的端口
    struct epoll_event* __revs; // 后面epoll继续的元素都会被放在这里
    int __revs_num; // __revs的大小
    using func_t = std::function<void(std::string)>; // io事件得到之后要处理的数据
    func_t __handler_request; // 如何处理数据，需要暴露给外面
public:
    epoll_server(func_t handler_request, const int& port = default_port)
        : __handler_request(handler_request)
        , __port(port)
        , __revs_num(g_revs_num) {
        // 0. 申请空间
        __revs = new struct epoll_event[__revs_num];
        // 1. 创建监听套接字
        __listen_sock = Sock::Socket();
        Sock::Bind(__listen_sock, __port);
        Sock::Listen(__listen_sock);
        // 2. 创建epoll模型
        __epoll_fd = __epoll::create_epoll();
        logMessage(DEBUG, "init success, listensock: %d, epfd: %d", __listen_sock, __epoll_fd); // 3, 4
        // 3. 先将listen套接字添加到epoll中，让epoll去管理！
        if (__epoll::control_epoll(__epoll_fd, EPOLL_CTL_ADD, __listen_sock, EPOLLIN)) {
            logMessage(DEBUG, "add listen sock to epoll success.");
        } else {
            exit(6);
        }
    }
    ~epoll_server() {
        if (__listen_sock >= 0)
            close(__listen_sock);
        if (__epoll_fd >= 0)
            close(__epoll_fd);
        if (__revs)
            delete[] __revs;
    }

public:
    void loop_once(int timeout) {
        /**
         * timeout表示，外部决定，到底这个epoll_wait最多阻塞多久，可以选择不阻塞，可以选择阻塞的事件
         */
        int n = __epoll::wait_epoll(__epoll_fd, __revs, __revs_num, timeout);
        switch (n) {
        case 0:
            logMessage(DEBUG, "timeout ...");
            break;
        case -1:
            logMessage(WARNING, "epoll wait error; %s", strerror(errno));
            break;
        default:
            // epoll这一次成功等到了就绪的fd了！
            logMessage(DEBUG, "get a event!"); // 有事件就绪啦
            handler_event(n);
            break;
        }
    }
    void start(int timeout) {
        while (true)
            loop_once(timeout);
    }

public:
    void epoll_accepter(int listen_sock) {
        // accept这个监听套接字
        std::string client_ip;
        uint16_t client_port;
        int accept_errno = 0;
        int sock = Sock::Accept(listen_sock, &client_ip, &client_port, &accept_errno);
        if (sock < 0) {
            logMessage(WARNING, "accept error!");
            return;
        }
        // 此时能不能直接读取？不能，因为并不清楚是否有数据！
        // 交给epoll！
        if (__epoll::control_epoll(__epoll_fd, EPOLL_CTL_ADD, sock, EPOLLIN)) {
            logMessage(DEBUG, "add new sock: %d to epoll", sock);
        } else {
            return;
        }
    }
    void epoll_recver(int sock) {
        // 读取这个普通套接字里面的内容
        // 1. 读取数据
        // 2. 处理数据
        char buffer[10240];
        size_t n = recv(sock, buffer, sizeof(buffer) - 1, 0);
        if (n > 0) {
            // 假设这里就是读到了一个完整的报文
            buffer[n] = 0;
            __handler_request(buffer); // 进行回调！
        } else if (n <= 0) {
            // 对端关闭文件描述符
            // 让epoll不再关注这个文件描述符
            // 一定要先从epoll中去掉，才能close文件描述符
            bool res = __epoll::control_epoll(__epoll_fd, EPOLL_CTL_DEL, sock, 0);
            assert(res); // 保证是成功的，因为一般来说都是成功的，所以直接assert
            (void)res;
            close(sock);
            if (n == 0)
                logMessage(NORMAL, "client %d quit, me quit too ...", sock);
            else if (n < 0)
                logMessage(NORMAL, "client recv %d error, close error sock", sock);
        }
    }
    void handler_event(int n) {
        assert(n > 0);
        for (int i = 0; i < n; i++) {
            uint32_t revents = __revs[i].events;
            int sock = __revs[i].data.fd; // 这个就是就绪（什么事件就绪呢，看下面）的文件描述符！
            // 此时如果我去对这个fd做访问，一定不会阻塞
            // 如果是listensock，就去accept，此时accept不会阻塞！
            // 如果是普通的sock，那我们就读取发过来的信息就好了！
            // 读事件就绪了！
            if (revents & EPOLLIN) {
                // 1. listensock 就绪
                // 2. 普通的sock就绪 - read
                if (sock == __listen_sock)
                    epoll_accepter(__listen_sock);
                else
                    epoll_recver(sock);
            }
        }
    }
};
} // namespace yufc

#endif