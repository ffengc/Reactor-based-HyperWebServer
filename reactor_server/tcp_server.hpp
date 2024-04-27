

#ifndef __YUFC_TCP_SERVER__
#define __YUFC_TCP_SERVER__

#include "log.hpp"
#include "poll.hpp"
#include "sock.hpp"
#include <assert.h>
#include <cerrno>
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
using business_func_t = std::function<void(connection*, std::string& request)>; // 上层的业务逻辑
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

private:
    business_func_t __callback_func; // 这个是上层的业务逻辑

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
    void dispather(business_func_t cb) {
        // 输入参数是上层的业务逻辑
        __callback_func = cb;
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
            // 将所有的异常，全部交给read和write来处理，所以异常直接打开in和out
            // read和write就会找except了！
            if (revents & EPOLLERR)
                revents |= (EPOLLIN | EPOLLOUT);
            if (revents & EPOLLHUP)
                revents |= (EPOLLIN | EPOLLOUT);
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
        // logMessage(DEBUG, "accepter is called");
        // 此时的listensock一定已经就绪了！
        // v1
        while (true) {
            std::string client_ip;
            uint16_t client_port;
            int accept_errno = 0;
            int sock = Sock::Accept(conn->__sock, &client_ip, &client_port, &accept_errno);
            // accept回来的sock就是正常io的sock
            if (sock < 0) {
                if (accept_errno == EAGAIN || accept_errno == EWOULDBLOCK) // 并不是出错了，是因为没链接了
                    break;
                else if (accept_errno == EINTR)
                    continue; // 概率非常低
                else {
                    logMessage(WARNING, "accept error, %d : %s", accept_errno, strerror(accept_errno));
                    break;
                }
            }
            // 将sock托管给poll和tcpserver
            __add_connection(sock, std::bind(&tcp_server::__recver, this, std::placeholders::_1),
                std::bind(&tcp_server::__sender, this, std::placeholders::_1),
                std::bind(&tcp_server::__excepter, this, std::placeholders::_1));
            logMessage(DEBUG, "accept client %s:%d success, add to epoll and server success", client_ip.c_str(), client_port);
        }
    }
    void __recver(connection* conn) {
        // 非阻塞读取，所以要循环读取
        // v1. 先面向字节流
        const int num = 1024;
        bool is_read_err = false;
        while (true) {
            char buffer[num];
            ssize_t n = recv(conn->__sock, buffer, sizeof(buffer) - 1, 0);
            if (n < 0) {
                if (errno == EAGAIN || errno == EWOULDBLOCK) // 读取完毕了(正常的break)
                    break;
                else if (errno == EINTR)
                    continue;
                else {
                    logMessage(ERROR, "recv error, %d:%s", errno, strerror(errno));
                    conn->__except_callback(conn); // 异常了，调用异常回调
                    is_read_err = true;
                    break;
                }
            } else if (n == 0) {
                logMessage(DEBUG, "client %d quit, server close %d", conn->__sock, conn->__sock);
                conn->__except_callback(conn);
                is_read_err = true;
                break;
            }
            // 读取成功了
            buffer[n] = 0;
            conn->__in_buffer += buffer; // 放到缓冲区里面就行了
        } // end while
        logMessage(DEBUG, "recv done, the inbuffer: %s", conn->__in_buffer.c_str());
        if (is_read_err == true)
            return;
        // 前面的读取没有出错
        // 这里就是上层的业务逻辑，如果对收到的报文做处理
        // 1. 切割报文，把单独的报文切出来
        // 2. 调用回调

        // 还没有想到怎么切割报文，这里假设读到的就是完整的报文
        __callback_func(conn, conn->__in_buffer);
    }
    void __sender(connection* conn) {
        while (true) {
            ssize_t n = send(conn->__sock, conn->__out_buffer.c_str(), conn->__out_buffer.size(), 0);
            if (n > 0) {
                conn->__out_buffer.erase(0, n);
                if (conn->__out_buffer.empty())
                    break; // 发完了
            } else {
                if (errno == EAGAIN || errno == EWOULDBLOCK)
                    break;
                else if (errno == EINTR)
                    continue;
                else {
                    logMessage(ERROR, "send error, %d:%s", errno, strerror(errno));
                    conn->__except_callback(conn);
                    break;
                }
            }
        }
        // 走到这里，要么就是发完，要么就是发送条件不满足，下次发送
        if (conn->__out_buffer.empty())
            enable_read_write(conn, true, false);
        else
            enable_read_write(conn, true, true);
    }
    void __excepter(connection* conn) {
        if (!is_sock_in_map(conn->__sock))
            return;
        // 1. 从epoll中移除
        if (!__poll.delete_from_epoll(conn->__sock))
            assert(false);
        // 2. 从map中移除
        __connection_map.erase(conn->__sock);
        // 3. close sock
        close(conn->__sock);
        // 4. delete conn
        delete conn;
        logMessage(DEBUG, "__excepter called");
    }

public:
    void enable_read_write(connection* conn, bool readable, bool writable) {
        uint32_t events = (readable ? EPOLLIN : 0) | (writable ? EPOLLOUT : 0);
        if (!__poll.control_poll(conn->__sock, events))
            logMessage(ERROR, "trigger write event fail");
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