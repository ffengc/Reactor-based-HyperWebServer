
#include "tcp_server.hpp"
#include <memory>

int main() {
    std::unique_ptr<yufc::tcp_server> svr(new yufc::tcp_server());
    svr->dispather();
    return 0;
}