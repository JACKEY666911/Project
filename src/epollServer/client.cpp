#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/epoll.h>
#include <error.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <string>
#include <iostream>
#include<jsoncpp/json/json.h>

using namespace Json;
using namespace std;

int main(int argc, char *argv[])
{
    int port = 8887;

    int socket_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if(socket_fd < 0)
    {
        printf("socket create err  ret = %d\n", socket_fd);
        exit(-1);
    }

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = inet_addr("192.168.4.55");
    
   int ret = connect(socket_fd, (struct sockaddr *)&addr, sizeof(addr));
    if(ret != 0)
    {
        printf("连接失败\n");
        exit(1);
    }
    
    Value root;
	FastWriter writer;
	root["code"] = 23021;
	root["flag"] = 15;
	root["mode"] = 1;
	root["username"] = "112451";
	root["password"] = "326578";
	string json = writer.write(root);
	cout << json;
    int sendsize = send(socket_fd, json.c_str(), json.size(), 0);

    int str_len;
    char mesg[1024];
    string Mesg;
    while(1)
    {
        memset(mesg, 0, sizeof(mesg));
        str_len = recv(socket_fd, mesg, 1024, 0);	
        if(str_len < 0)
        {
            break;
        }
        else if(str_len == 0)//客户端信息传送完毕，断开连接
        {
            close(socket_fd);
            break;
        }
        else if(str_len == sizeof(mesg))
        { 
            //cout << Mesg << " size" << Mesg.size();
            Mesg.append(mesg);	
        }
        else if(0 < str_len < sizeof(mesg))
        {
            Mesg.append(mesg);	
            break;
        }
    }
    Value results;
    Reader reader;
    reader.parse(Mesg, results);
    cout << "Mesg: " << Mesg << endl;

    sleep(20);
	close(socket_fd);
    return 0;
}
