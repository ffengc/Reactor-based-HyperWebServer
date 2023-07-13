
#ifndef __REACTOR__SERVER_HPP__
#define __REACTOR__SERVER_HPP__

#include <iostream>
#include <string>
#include <functional>
#include "Sock.hpp"
#include "Log.hpp"
#include "Epoll.hpp"
#include "Protocol.hpp"
#include <unordered_map>

class Connection;
class ReactorServer;
using func_t = std::function<void(Connection *)>;
using callback_t = std::function<void(Connection *, std::string &request)>;
// 为了能够正常工作，常规的sock必须要有自己的接收缓冲区&&发送缓冲区
class Connection
{
public:
    Connection(int sock = -1) : __sock(sock), __tsvr(nullptr)
    {
    }
    ~Connection() {}
    void SetCallback(func_t recv_cb, func_t send_cb, func_t except_cb)
    {
        // 通过这个函数可以设置回调方法（可以直接设置，因为是public，不过为这里还是单独为三个回调设置一个Set方法吧）
        this->__recv_cb = recv_cb;
        this->__send_cb = send_cb;
        this->__except_cb = except_cb;
    }

public:
    int __sock;       // 负责进行IO的文件描述符
    func_t __recv_cb; // 读回调
    func_t __send_cb; // 写回调
    func_t __except_cb;
    // 三个回调方法，表征的就是对__sock进行特定读写对应的方法
    // 接收&&发送缓冲区
    std::string __in_buffer; // 暂时没有办法处理二进制流，文本是可以的
    std::string __out_buffer;
    // 设置对ReactorServer的回指指针
    ReactorServer *__tsvr;
    // 设置时间戳
    uint64_t __last_time_stamp; // 上一次访问的时间，如果太长，直接给你断了
};

class ReactorServer
{
public:
    const static int __gport = 8080;
    const static int __gnum = 128;
private:
    int __listen_sock;
    int __port;
    Epoll __poll;
    std::unordered_map<int, Connection *> __connections_map; // sock : Connection
    struct epoll_event *__revs;
    int __revs_num;
    // 这里是上层的业务处理
    callback_t __cb;
public:
    void EnableReadWrite(Connection *conn, bool readable, bool writeable)
    {
        uint32_t events = (readable ? EPOLLIN : 0) | (writeable ? EPOLLOUT : 0);
        bool res = __poll.CtrlEpoll(conn->__sock, events);
        if (!res)
            logMessage(ERROR, "Modify sock's epoll error, %d:%s", errno, strerror(errno));
    }
private:
    // 设计一个方法，专门针对人意sock，添加到ReactorServer中来
    void __add_connection(int sock, func_t recv_cb, func_t send_cb, func_t except_cb)
    {
        /* 意思就是：针对于这个sock，我的读取回调，写回调，其他回调，是这么些东西，
            到时候sock就绪了，去调相应的方法就行！*/
        // 0. 先把这个sock弄成非阻塞的sock，因为要用ET模式
        Sock::SetNonBlock(sock);
        // 1. 构建conn对象，封装sock
        Connection *conn = new Connection(sock);
        conn->SetCallback(recv_cb, send_cb, except_cb);
        conn->__tsvr = this; // 让这connection指向自己
        // 2. 把sock添加到epoll里
        // 任何多路转接服务器，一般只会默认打开对读取时间的关心，写入事件会按需进行打开！
        __poll.AddSockToEpoll(sock, EPOLLIN | EPOLLET);
        // 3. 把对应的Connection*对象指针添加到map里面
        __connections_map.insert(std::make_pair(sock, conn));
    }
    bool __if_sock_in_map(int sock)
    {
        auto iter = __connections_map.find(sock);
        if (iter == __connections_map.end())
            return false;
        return true;
    }
private:
    void __accepter(Connection *conn)
    {
        // 走到这里，代表肯定是有一个listensock就绪了，此次读取一定不会阻塞，这也不用说了
        while (true)
        {
            // 为什么要循环读取？因为我们是ET模式下工作？难道底层链接只有一个吗？Accept一次就够了吗？
            std::string client_ip;
            uint16_t client_port;
            int accept_errno = 0;
            int sock = Sock::Accept(conn->__sock, &client_ip, &client_port, &accept_errno); // 后面两个是输出型参数，我们今天就不关心了
            // 后面两个参数是客户端的信息，我们今天就不关心了
            if (sock < 0)
            {
                if (accept_errno == EAGAIN || accept_errno == EWOULDBLOCK)
                    break; // 底层没有链接
                else if (accept_errno == EINTR)
                    continue; // 概率非常低
                else
                {
                    logMessage(WARNING, "accept error, %d: %s", accept_errno, strerror(accept_errno));
                    break;
                }
            }
            // 获取成功
            // 将新的sock托管给ReactorServer
            if (sock >= 0)
            {
                this->__add_connection(sock, std::bind(&ReactorServer::__recver, this, std::placeholders::_1),
                                       std::bind(&ReactorServer::__sender, this, std::placeholders::_1),
                                       std::bind(&ReactorServer::__excepter, this, std::placeholders::_1));
                logMessage(DEBUG, "accept client %s:%d success, add to epoll&&ReactorServer", client_ip.c_str(), client_port);
            }
        }
    }
    void __recver(Connection *conn)
    {
        // logMessage(DEBUG, "__recver() is called");
        const int num = 1024;
        bool err = false;
        while (true)
        {
            // 配合协议
            char buffer[num] = {0};
            ssize_t n = recv(conn->__sock, buffer, sizeof(buffer) - 1, 0);
            if (n < 0)
            {
                if (errno == EAGAIN || errno == EWOULDBLOCK) // 走到这里break是正常的
                    break;
                else if (errno == EINTR)
                    continue;
                else
                {
                    // 读取失败
                    logMessage(ERROR, "recv error, %d : %s", errno, strerror(errno));
                    conn->__except_cb(conn); // 出错了 --> 交给异常回调去处理！！！！
                    err = true;              // 表示出错了
                }
            }
            else if (n == 0)
            {
                // 对面把链接关了
                logMessage(DEBUG, "client quit, server close");
                conn->__except_cb(conn);
                err = true;
                break;
            }
            else
            {
                // 读取成功
                buffer[n] = '\0';
                conn->__in_buffer += buffer; // 把数据放到缓冲区里就行了！这个buffer就是一个http报文，交给上层处理
            }
        } // end while
        logMessage(DEBUG, "conn->__in_buffer[sock: %d]: %s", conn->__sock, conn->__in_buffer.c_str());
        logRequest(conn->__in_buffer.c_str());

        // 这里应该怎么写，还没解决？
        if (!err)
        {
            this->__cb(conn, conn->__in_buffer); // 这里认为直接可以读取到一个完整的http报文
        }
    }
    void __sender(Connection *conn)
    {
        logMessage(DEBUG, "__sender() is called");
        // 在这里我们认为，发送的数据已经堆到outbuffer里面了
        while (true)
        {
            ssize_t n = send(conn->__sock, conn->__out_buffer.c_str(), conn->__out_buffer.size(), 0);
            // 期望全部把out发送出去，但是事实上不一定可以！
            if (n > 0)
            {
                conn->__out_buffer.erase(0, n); // 发送成功的outbuffer里面直接移除就行了
                if (conn->__out_buffer.empty())
                    break;
            }
            else if (n == 0)
            {
                if (errno == EAGAIN || errno == EWOULDBLOCK)
                    break;
                else if (errno == EINTR)
                    continue;
                else
                {
                    logMessage(ERROR, "sender error, %d:%s", errno, strerror(errno));
                    conn->__except_cb(conn);
                    break;
                }
            }
        }
        // 发完了吗，走到这里发完了吗？不确定，我们可以让他给我进行统一处理
        // 但是我们能保证，如果没有出错，要么一定发完，要么就是发送条件不满足，下次发送
        if (conn->__out_buffer.empty())
        {
            // 如果这一轮把东西发完了！直接关闭写事件！
            EnableReadWrite(conn, true, false);
        }
        else
        {
            EnableReadWrite(conn, true, true);
        }
    }
    void __excepter(Connection *conn)
    {
        // logMessage(DEBUG, "__excepter() is called");
        if (!__if_sock_in_map(conn->__sock))
            return;
        // 如果这个sock不在里面，说明之前一早就处理过了，这里不用处理了！
        // 1. 从epoll中移除这个sock
        // 2. delete conn对象
        // 3. close 关闭对应的sock
        // 4. 从我们的unordered_map中移除
        __poll.DelFromEpoll(conn->__sock);
        __connections_map.erase(conn->__sock);
        close(conn->__sock);
        delete conn;
        logMessage(DEBUG, "Excepter done");
    }

public:
    ReactorServer(int port = __gport) : __port(port), __revs_num(__gnum)
    {
        // 0. 初始化日志文件
        InitLogFile(__LOGBEGIN__);
        // 1. 创建listensock
        __listen_sock = Sock::Socket();
        Sock::Bind(__listen_sock, __port);
        Sock::Listen(__listen_sock);
        // 2. 创建多路转接对象
        __poll.CreateEpoll();
        // 3. __add_connection
        this->__add_connection(__listen_sock, std::bind(&ReactorServer::__accepter, this, std::placeholders::_1), nullptr, nullptr);
        // 服务器中存在大量Connection的时候，我们的服务器就要对这些Connection进行管理

        // 4. 构建一个获取就绪事件的缓冲区
        // 即就绪事件，都要放在这里！
        __revs = new struct epoll_event[__revs_num];
    }
    ~ReactorServer()
    {
        InitLogFile(__LOGEND__);
        if (__listen_sock >= 0)
            close(__listen_sock);
        if (__revs)
            delete[] __revs;
    }
public:
    void LoopOnce()
    {
        // std::cout << "LoopOnce called" << std::endl; // for debug
        // exit(-1); // for debug
        int n = __poll.WaitEpoll(__revs, __revs_num);
        for (int i = 0; i < n; i++)
        {
            int sock = __revs[i].data.fd;
            uint32_t revents = __revs[i].events;
            // 将所有的异常，全部交给read和write来处理，read和write看到你出错了，都会给你丢到异常处理里面去
            if (revents & EPOLLERR)
                revents |= (EPOLLIN | EPOLLOUT);
            if (revents & EPOLLHUP)
                revents |= (EPOLLIN | EPOLLOUT);
            if (revents & EPOLLIN)
            {
                if (__if_sock_in_map(sock) && __connections_map[sock]->__recv_cb != nullptr)
                {
                    // 表示这个方法读就绪，而且是在map里面，而且！他是有读回调的，即读就绪是要去处理的！
                    __connections_map[sock]->__recv_cb(__connections_map[sock]); // 调用这个回调！
                }
            }
            if (revents & EPOLLOUT)
            {
                if (__if_sock_in_map(sock) && __connections_map[sock]->__send_cb != nullptr)
                {
                    __connections_map[sock]->__send_cb(__connections_map[sock]);
                }
            }
        }
    }
    void SetCallBack(callback_t cb)
    {
        this->__cb = cb;
    }
};
#endif