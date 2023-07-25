#include"httpserver.hpp"
#include"sqlite.hpp"

int main()
{
    //设置端口号
    int port = 8080;
    //设置IP地址
    const char *address = "192.168.4.55";


    HttpServer server;
    server.start(address, port);
}
