
/* 47.120.41.234 */

#ifndef __WebServer_HPP__
#define __WebServer_HPP__

#include "Reactor/Reactor-server.hpp"
#include "tools/ulity.hpp"
#include <memory>
#include <iostream>
#include <fstream>

/* 一般http都要有自己的web根目录 */
#define ROOT "./wwwroot"
/* 如果客户端只请求了一个/ ，一般返回默认首页 */
#define HOME_PAGE "index.html"

class WebServer
{
private:
    std::unique_ptr<ReactorServer> __reactor_svr_ptr;
public:
    WebServer(int port = ReactorServer::__gport)
    {
        __reactor_svr_ptr = std::unique_ptr<ReactorServer>(new ReactorServer(port));
        __reactor_svr_ptr->SetCallBack(Respones);
    }
    void Dispather()
    {
        // 根据就绪的事件，进行特定事件的派发
        while (true)
        {
            this->__reactor_svr_ptr->LoopOnce();
        }
    }
    static std::string AnalyzeHTTPMessages(std::string &request)
    {
        std::vector<std::string> vline;
        Util::cutString(request, "\r\n", &vline);
        std::vector<std::string> vblock;
        Util::cutString(vline[0], " ", &vblock); // 因为我们要提取到底这个请求要什么资源
        /* http的报文 第一行就告诉了我们到底这个请求需要什么资源 所以把第一行再分一下就行 */
        std::string file = vblock[1]; // 按照报文的格式，第一行第二个字符串，就是资源路径
        std::string target = ROOT;
        if (file == "/")
            file = "/index.html";
        target += file;
        return target;
    }
    static std::string BuildHTTPResponseMessage(std::string &target)
    {
        std::string content;
        std::ifstream in(target);
        if (in.is_open())
        {
            std::string line;
            while (std::getline(in, line))
            {
                content += line;
            }
            in.close();
        }
        std::string HttpResponse;
        if (content.empty())
        {
            HttpResponse = "HTTP/1.1 200 OK\r\n";
            std::string content_404;
            std::string target_temp;
            target_temp += "./wwwroot/error/404.html";
            std::ifstream in(target_temp);
            if (in.is_open())
            {
                std::string line;
                while (std::getline(in, line))
                {
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
    static void Respones(Connection *conn, std::string &request)
    {
        // 处理http报文
        std::string target = WebServer::AnalyzeHTTPMessages(request);
        // 2. 构建一个Http构建一个响应
        std::string HttpResponse = WebServer::BuildHTTPResponseMessage(target);
        // 3. 放到conn到发送缓冲区中
        conn->__out_buffer += HttpResponse;
        // 4. 调用send
        conn->__tsvr->EnableReadWrite(conn, true, true);
    }
};

#endif