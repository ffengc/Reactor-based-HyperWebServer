

#ifndef __YUFC_SOCK__
#define __YUFC_SOCK__

#include "log.hpp"
#include <arpa/inet.h>
#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <iostream>
#include <netinet/in.h>
#include <pthread.h>
#include <signal.h>
#include <string.h>
#include <string>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

class Sock {
private:
    const static int gbacklog = 10; // 一般不能太大也不能太小，后面再详细解释
public:
    Sock() { }
    ~Sock() { }
    static int Socket() {
        int listen_sock = socket(AF_INET, SOCK_STREAM, 0);
        if (listen_sock < 0) {
            logMessage(FATAL, "create socket error, %d: %s", errno, strerror(errno));
            exit(2);
        }
        int opt = 1;
        setsockopt(listen_sock, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof opt);
        logMessage(NORMAL, "create socket success, sock: %d", listen_sock);
        return listen_sock;
    }
    static void Bind(int sock, uint16_t port, std::string ip = "0.0.0.0") {
        struct sockaddr_in local;
        memset(&local, 0, sizeof(local));
        local.sin_family = AF_INET;
        local.sin_port = htons(port);
        inet_pton(AF_INET, ip.c_str(), &local.sin_addr);
        if (bind(sock, (struct sockaddr*)&local, sizeof(local)) < 0) {
            logMessage(FATAL, "bind error, %d: %s", errno, strerror(errno));
            exit(3);
        }
    }
    static void Listen(int sock) {
        if (listen(sock, gbacklog) < 0) {
            logMessage(FATAL, "listen error, %d: %s", errno, strerror(errno));
            exit(4);
        }
        logMessage(NORMAL, "init TcpServer Success");
    }
    static int Accept(int listensock, std::string* ip, uint16_t* port, int* accept_errno) {
        /*
        一般经验:
        const std::string& 输入型参数
        std::string * 输出型参数
        std::string & 输入输出型参数
        */
        struct sockaddr_in src;
        socklen_t len = sizeof(src);
        *accept_errno = 0;
        int service_sock = accept(listensock, (sockaddr*)&src, &len);
        // 它的返回值也是一个套接字
        if (service_sock < 0) {
            // logMessage(ERROR, "accept error, %d: %s", errno, strerror(errno));
            // 不一定是出错了，有可能是ET模式下底层没有数据了
            *accept_errno = errno;
            return -1;
            // 获取服务连接失败，并不需要崩掉，而是继续获取！
        }
        // 获取连接成功
        if (port)
            *port = ntohs(src.sin_port);
        if (ip)
            *ip = inet_ntoa(src.sin_addr);
        logMessage(NORMAL, "link success, serviceSock: %d | %s : %d", service_sock, ip->c_str(), port);
        return service_sock;
    }
    static bool Connect(int sock, const std::string& server_ip, const uint16_t& server_port) {
        struct sockaddr_in server;
        memset(&server, 0, sizeof server);
        server.sin_family = AF_INET;
        server.sin_port = htons(server_port);
        server.sin_addr.s_addr = inet_addr(server_ip.c_str());

        if (connect(sock, (struct sockaddr*)&server, sizeof(server)) == 0)
            return true;
        else
            return false;
    }
    static bool SetNonBlock(int sock) {
        int fl = fcntl(sock, F_GETFL);
        if (fl < 0)
            return false;
        fcntl(sock, F_SETFL, fl | O_NONBLOCK);
        return true;
    }
};
#endif