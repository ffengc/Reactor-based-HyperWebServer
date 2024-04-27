
#ifndef __YUFC_UTIL__
#define __YUFC_UTIL__

#include <string>
#include <vector>

class Util {
public:
    /* 提取http请求报文的每一行 放到out里面去 */
    static void cut_string(const std::string& s, const std::string& sep, std::vector<std::string>* out) {
        /* 把每一行提取出来 */
        std::size_t start = 0;
        while (start < s.size()) {
            auto pos = s.find(sep, start);
            if (pos == std::string::npos) {
                break;
            }
            std::string sub = s.substr(start, pos - start);
            out->push_back(sub);
            start += sub.size();
            start += sep.size();
        }
        // 读到最后之后，如果最后没有分隔符了，但是后面还是有内容的
        if (start < s.size()) {
            out->push_back(s.substr(start));
        }
    }
};

#endif