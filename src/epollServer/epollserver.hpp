#ifndef EPOLLSERVER_HPP
#define EPOLLSERVER_HPP

#include <sys/epoll.h>
#include"sqlite.hpp"
#include"XthreadPool.hpp"
#include"TaskQueue.hpp"


#define MESG_SIZE 1024
#define EVENT_SIZE 50
#define ERR -1
#define ERRHAND(FD,ERRMSG) do{if(FD < 0){printf("something is error!\n");perror(ERRMSG);exit(1);}}while(0)
#define ERRHAND_CON(FD,ERRMSG) do{if(FD < 0){printf("something is error!\n");perror(ERRMSG);continue;}}while(0)

//flag
#define REGISTER_LOGIN 0xF
//mode
#define REGISTER 0x1
#define LOGIN 0x2

//flag
#define USER_MANAGE 0xE
//mode
#define USER_INSERT 0x1
#define USER_QUERY 0x2
#define USER_DELETE 0x3
#define USER_REVISE 0x4

#define CODE_1 23021
#define CODE_2 23020

#define SQLITE_ERR "-1"
#define FOUND_ERR "0"
#define SUCESS "1"
#define ERRPASSWORD "2"
#define GREATPASSWORD "3"

#define ERRUSERNAME "1"

using namespace std;
using namespace Json;

int setnonblocking(int fd);
void getJsonMesg(const int fd, const int epfd, Value &results);
void forQtDEvice(int fd, const Value & results);
void forQtClient(int fd, const Value & results);
void register_login(const int fd, const Value &results);
void user_manage(const int fd, const Value &results);


class eopllServer
{

public:
    eopllServer();
    ~eopllServer();
    void listen_init(const char *address, const int port);
    void accpet_start();

protected:
    int tcp_sockfd;
    int epfd;
};

struct arg
{
    arg(int fd_, Value results_)
    {
        fd      = fd_;
        results = results_;
    }
    int fd;
    Value results;
};


#endif