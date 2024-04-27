
#include "web_server.hpp"
#include <memory>

int main() {
    std::unique_ptr<yufc::web_server> svr(new yufc::web_server());
    svr->start();
    return 0;
}