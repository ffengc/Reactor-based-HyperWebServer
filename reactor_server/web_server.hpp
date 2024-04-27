

#ifndef __WebServer_HPP__
#define __WebServer_HPP__

#include "tcp_server.hpp"
#include "util.hpp"
#include <fstream>
#include <iostream>
#include <memory>

#define ROOT "./wwwroot" /* 一般http都要有自己的web根目录 */
#define HOME_PAGE "index.html" /* 如果客户端只请求了一个/ ，一般返回默认首页 */
#define PORT 8080

namespace yufc {

class web_server {
private:
    std::unique_ptr<yufc::tcp_server> __reactor;

public:
    web_server(int port = PORT) {
        __reactor = std::unique_ptr<yufc::tcp_server>(new yufc::tcp_server(port));
        // 还需要设置回调
    }
    ~web_server() = default;
    void start() {
        __reactor->dispather(respones);
    }
    static std::string analyze_http_mesg(std::string& request) {
        std::vector<std::string> vline;
        Util::cut_string(request, "\r\n", &vline);
        std::vector<std::string> vblock;
        Util::cut_string(vline[0], " ", &vblock);
        std::string file = vblock[1];
        std::string target = ROOT;
        if (file == "/")
            file = "/index.html";
        target += file;
        return target;
    }
    static std::string build_http_response_mesg(std::string& target) {
        std::string content;
        std::ifstream in(target);
        if (in.is_open()) {
            std::string line;
            while (std::getline(in, line)) {
                content += line;
            }
            in.close();
        }
        std::string HttpResponse;
        if (content.empty()) {
            HttpResponse = "HTTP/1.1 200 OK\r\n";
            std::string content_404;
            std::string target_temp;
            target_temp += "./wwwroot/error/404.html";
            std::ifstream in(target_temp);
            if (in.is_open()) {
                std::string line;
                while (std::getline(in, line)) {
                    content_404 += line;
                }
                in.close();
            }
            content = content_404;
        }

        else
            HttpResponse = "HTTP/1.1 200 OK\r\n";
        assert(!content.empty());
        HttpResponse += "\r\n";
        HttpResponse += content;
        return HttpResponse;
    }
    static void respones(yufc::connection* conn, std::string& request) {
        // 处理http报文
        std::string target = web_server::analyze_http_mesg(request);
        // 2. 构建一个Http构建一个响应
        std::string HttpResponse = web_server::build_http_response_mesg(target);
        // 3. 放到conn到发送缓冲区中
        conn->__out_buffer += HttpResponse;
        // 4. 调用send
        conn->__tsvr->enable_read_write(conn, true, true);
    }
};

} // namespace yufc

#endif
