

#ifndef __Yufc_Protocol_For_Cal
#define __Yufc_Protocol_For_Cal

#include <iostream>
#include <string>
#include <string.h>
#include <vector>

// 支持解决粘报问题，处理独立报文
#define SEP "X"
#define SEP_LEN strlen(SEP)

// 自己手写序列反序列化
#define SPACE " "
#define SPACE_LEN strlen(SPACE)

#define RECV_MAX_SIZE 1024

/* 请求协议是必须自带序列化功能的 一个结构体是很难发送的 需要序列化成string */

#define MYSELF true
#define SPACE " "
#define SPACE_LEN strlen(SPACE)
class Request
{
public:
    std::string Serialize()
    {
        std::string str;
        str = std::to_string(__x);
        str += SPACE;
        str += __op; // BUG
        str += SPACE;
        str += std::to_string(__y);
        return str;
    }
    bool Deserialize(const std::string &str)
    {
        std::size_t left = str.find(SPACE);
        if (left == std::string::npos)
        {
            return false; // 反序列化失败
        }
        std::size_t right = str.rfind(SPACE);
        if (right == std::string::npos)
        {
            return false; // 反序列化失败
        }
        __x = atoi(str.substr(0, left).c_str()); // 拿到__x的字符串之后直接 atoi 就行
        __y = atoi(str.substr(right + SPACE_LEN).c_str());
        if (left + SPACE_LEN > str.size())
            return false;
        __op = str[left + SPACE_LEN];
        return true;
    }
public:
    Request() {}
    Request(int x, int y, char op) : __x(x), __y(y), __op(op) {}
    ~Request() {}
public:
    int __x;
    int __y;
    char __op; // '+' ...
};

class Response
{
public:
    /* "code result" */
    std::string Serialize()
    {
        std::string s;
        s = std::to_string(__result);
        s += SPACE;
        s += std::to_string(__code);
        return s;
    }
    bool Deserialize(const std::string &s)
    {
        // std::cerr << "s: " << s << std::endl;
        std::size_t pos = s.find(SPACE);
        if (pos == std::string::npos)
            return false;
        __code = atoi(s.substr(0, pos).c_str());
        __result = atoi(s.substr(pos + SPACE_LEN).c_str());
        return true;
    }
public:
    Response() {}
    Response(int result, int code) : __result(result), __code(code) {}
    ~Response() {}
public:
    int __result; // 计算结果
    int __code;   // 计算结果的状态码
    /* 0表示计算结果可信任 */
};


void SpliteMessage(std::string &buffer, std::vector<std::string> *out)
{
    // 100+
    // 100+19X1
    // 100+19X100+19
    while(true)
    {
        auto pos = buffer.find(SEP);
        if(std::string::npos == pos) break;
        std::string message = buffer.substr(0, pos);
        buffer.erase(0, pos + SEP_LEN);
        out->push_back(message);
    }
}

#endif