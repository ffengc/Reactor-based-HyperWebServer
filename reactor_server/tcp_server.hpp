

#ifndef __YUFC_TCP_SERVER__
#define __YUFC_TCP_SERVER__

#include "log.hpp"
#include "poll.hpp"
#include "sock.hpp"
#include <functional>
#include <iostream>
#include <string>
#include <unordered_map>

namespace yufc {
/**
 * 为了能够正常工作，常规的sock必须是要有自己独立的接收缓冲区和发送缓冲区！
 */

class tcp_server;
class connection;
using func_t = std::function<void(connection*)>;
class connection {
public:
    connection(int sock = -1)
        : __sock(sock)
        , __tsvr(nullptr) { }
    ~connection() { }
    void set_callback(func_t recv_cb, func_t send_cb, func_t except_cb) {
        __recv_callback = recv_cb;
        __send_callback = send_cb;
        __except_callback = except_cb;
    }

public:
    int __sock; // io的文件描述符
    func_t __recv_callback;
    func_t __send_callback;
    func_t __except_callback;
    std::string __in_buffer; // 输入缓冲区（暂时没有处理二进制流）
    std::string __out_buffer; // 输出缓冲区
    tcp_server* __tsvr; // 回指指针
};

class tcp_server {
public:
    const static int gport = 8080;
    const static int gnum = 128;

private:
    int __listen_sock;
    int __port;
    __epoll __poll; // 这里直接维护一个epoll
    std::unordered_map<int, connection*> __connection_map;
    struct epoll_event* __revs;
    int __revs_num;

public:
    tcp_server(int port = gport)
        : __port(gport)
        , __revs_num(gnum)
        , __poll(0) { /* 这里给poll设置非阻塞 */
        // 1. 创建listensock
        __listen_sock = Sock::Socket();
        Sock::Bind(__listen_sock, __port);
        Sock::Listen(__listen_sock);
        // 2. 创建多路转接对象
        __poll.create_poll();
        // 3. 添加listensock到tcp_server::__connection_map和poll中
        __add_connection(__listen_sock, std::bind(&tcp_server::__accepter, this, std::placeholders::_1), nullptr, nullptr);
        // 4. 构建一个获取就绪事件的缓冲区
        __revs = new struct epoll_event[__revs_num];
    }
    ~tcp_server() {
        if (__listen_sock >= 0)
            close(__listen_sock);
        if (__revs)
            delete[] __revs;
    }

public:
    void dispather() {
        while (true) {
            loop_once();
        }
    }
    void loop_once() {
        // 捞取所有就绪事件到revs数组中
        int n = __poll.wait_poll(__revs, __revs_num);
        for (int i = 0; i < n; i++) {
            // 此时就可以去处理已经就绪事件了！
            int sock = __revs[i].data.fd;
            uint32_t revents = __revs[i].events;
            if (revents & EPOLLIN) {
                // 这个事件读就绪了
                // 1. 先判断这个套接字是否在这个map中存在
                if (is_sock_in_map(sock) && __connection_map[sock]->__recv_callback != nullptr)
                    __connection_map[sock]->__recv_callback(__connection_map[sock]);
            }
            if (revents & EPOLLOUT) {
                if (is_sock_in_map(sock) && __connection_map[sock]->__send_callback != nullptr)
                    __connection_map[sock]->__send_callback(__connection_map[sock]);
            }
        }
    }

private:
    void __add_connection(int sock, func_t recv_cb, func_t send_cb, func_t except_cb) {
        // 不同种类的套接字都可以调用这个方法
        // 0. ！先把sock弄成非阻塞！
        Sock::SetNonBlock(sock);
        // 1. 构建conn对象，封装sock
        connection* conn = new connection(sock);
        conn->set_callback(recv_cb, send_cb, except_cb);
        conn->__tsvr = this; // 让conn对象指向自己
        // 2. 添加sock到poll中
        __poll.add_sock_to_poll(sock, EPOLLIN | EPOLLET);
        // 3. 把封装好的conn放到map里面去
        __connection_map.insert({ sock, conn });
    }
    void __accepter(connection* conn) {
        logMessage(DEBUG, "accepter is called");
    }

public:
    bool is_sock_in_map(int sock) {
        auto iter = __connection_map.find(sock);
        if (iter == __connection_map.end())
            return false;
        return true;
    }
};
}

#endif