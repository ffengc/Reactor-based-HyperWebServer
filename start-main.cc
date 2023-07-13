

#include "WebServer.hpp"

int main()
{
    std::unique_ptr<WebServer> svr(new WebServer());
    svr->Dispather();
    return 0;
}