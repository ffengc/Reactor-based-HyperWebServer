
#include "epoll_server.hpp"
#include <memory>

void change(std::string request) {
    // 完成业务逻辑
    std::cout << "change: " << request << std::endl;
}

int main() {
    std::unique_ptr<ns_epoll::epoll_server> svr(new ns_epoll::epoll_server(change));
    svr->start(-1); // 先暂时让他阻塞等待
    return 0;
}