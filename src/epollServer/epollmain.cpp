#include "epollserver.hpp"
#include"sqlite.hpp"
#include<iostream>
using namespace std;
using namespace Json;
int main()
{
    //ThreadPool *pool = new ThreadPool(3, 8);
    //string dbname("test5.db");
    //emplyInfo emply(dbname);
    //emply.CreateForm();
    //设置端口号
    int port = 8887;
    //设置IP地址
    const char *address = "192.168.4.55";
    eopllServer eopllserver;
    eopllserver.listen_init(address, port);
    eopllserver.accpet_start();
    //delete pool;
    return 0;
}