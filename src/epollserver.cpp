#include<stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <arpa/inet.h>
#include<string.h>
#include<unistd.h>
#include <sys/epoll.h>
#include <fcntl.h>
#include<errno.h>

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

int main(int argc, const char *argv[])
{
	int tcp_sockfd = 0;
	tcp_sockfd = socket(AF_INET, SOCK_STREAM, 0);
	ERRHAND(tcp_sockfd,"sockect");

	struct sockaddr_in serverAddr,clientAddr;
	socklen_t cliLength = 0;
	socklen_t serLength = sizeof(serverAddr);

	memset(&serverAddr, 0, serLength);
	memset(&clientAddr, 0, cliLength);

	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(8887); 
	//serverAddr.sin_port = htons(atoi(argv[2])); 
	//serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	//serverAddr.sin_addr.s_addr = inet_addr("192.168.4.56");//此函数会将字符串形式的IP地址转换程32位整数
    inet_aton("192.168.1.250", &serverAddr.sin_addr);
	/*
	函数原型为char *inet aton(const char *string, struct in_addr *adr);
	作用是将网络字节序转换成字符串类型
	*/
	/*
	初始化套接字，将套接字绑定IP地址和端口
	*/
	int ret = 0;
	ret = bind(tcp_sockfd, (struct sockaddr*)&serverAddr, serLength);
	ERRHAND(ret, "bind");
	ret = listen(tcp_sockfd, 10);
	ERRHAND(ret, "listen");
	
	int fd;
	int epfd;
	struct epoll_event ev;
	ev.events = EPOLLIN;
	ev.data.fd = tcp_sockfd;

	int epoll_cnt;
	struct epoll_event epevents[EVENT_SIZE] = {0};
	int event_size = sizeof(epevents);

	epfd = epoll_create(1);
	if(epfd == -1)
	{
		perror("epoll_create");
		exit(1);
	}

    ret = epoll_ctl(epfd, EPOLL_CTL_ADD , tcp_sockfd, &ev);//添加服务器端套接字
	ERRHAND(ret, "epoll_ctl");
	
	//printf("1111111111111111\n");

	while(1)//迭代服务器，处理一个客户端的数据传递后，继续受理后续的客户端
	{
		epoll_cnt = epoll_wait(epfd, epevents, event_size, -1);
		printf("epoll_cnt:%d\n",epoll_cnt);
		ERRHAND(epoll_cnt, "epoll_wait");

		for(int i = 0; i < epoll_cnt; ++i)
		{
			fd = epevents[i].data.fd;
			setnonblocking(fd);//将套接字（文件描述符设置为非阻塞I/O）

			if(fd == tcp_sockfd)
			{
				int accept_sock = accept(fd, (struct sockaddr *)&clientAddr, &cliLength);
				ERRHAND(ret, "accept");
				char *str_clientAddr = inet_ntoa(clientAddr.sin_addr);
				int clientPort = ntohs(clientAddr.sin_port);
				printf("有人连接服务器，IP地址为%s,端口号为%d\n",str_clientAddr,clientPort);

				struct epoll_event accept_ev;
				accept_ev.events = EPOLLIN | EPOLLET;//将epoll事件设置为边沿触发模式
				accept_ev.data.fd = accept_sock;

				ret = epoll_ctl(epfd, EPOLL_CTL_ADD, accept_sock, &accept_ev);
				ERRHAND_CON(ret, "epoll_ctl");
			}
			else
			{
				Value results;
				getJsonMesg(fd, epfd, results);
				switch (results["code"].asInt())
				{
					case CODE_1:
						forQtClient(fd, results);
						break;
					case CODE_2:
						forQtDEvice(fd, results);
						break;				
					default:
						break;
				}
			}
		}
	}

	close(tcp_sockfd);
	close(epfd);
	return 0;
}

void getJsonMesg(const int fd, const int epfd, Value &results)
{
	Reader reader;
	char *mesg = new char[MESG_SIZE];
	memset(mesg, 0, MESG_SIZE);
	int str_len = 0;
	/*在epoll边缘触发的条件下，并将read事件对应的文件描述符设置为非阻塞I/O模式，
		这时用户设置的用户空间缓冲区可以不用太大，也可以减少epoll的检测次数，得用while循环把recv缓冲区数据读完
	*/
	while(1)
	{
		str_len = recv(fd, mesg, MESG_SIZE, 0);
		if(str_len == -1)
		{
			if(errno == EAGAIN)//recv函数中，如果错误码为EAGAIN，意味着在非阻塞I/O的前提下，数据被读完了
			{
				printf("数据接受完毕...\n");
				break;
			}
			else
			{
				perror("recv");
				break;
			}
		}
		else if(str_len == 0)//客户端信息传送完毕，断开连接
		{
			epoll_ctl(epfd, EPOLL_CTL_DEL, fd, NULL);//删除事件节点时，第四个参数为NULL
			close(fd);
			//printf("有客户端断开连接,disconnected client:%d\n",epevents[i].data.fd);
			break;
		}
		else
		{ 
			write(STDOUT_FILENO, mesg, sizeof(mesg));
			reader.parse(mesg, results);
			memset(mesg, 0, sizeof(mesg));
		}
	}

}


void forQtDEvice(int fd, const Value & results)
{
	if(results["code"].asInt() != CODE_2)
	{
		printf("非法请求\n");
		return ;
	}

	return ;
}


void forQtClient(int fd, const Value & results)
{
	if(results["code"].asInt() != CODE_1)
	{
		printf("非法请求\n");
		return ;
	}

	if(results["flag"].asInt() == REGISTER_LOGIN)
	{	
		register_login(fd, results);
	}
	else if(results["flag"].asInt() == USER_MANAGE)
	{
		user_manage(fd, results);	
	}

	return ;
}


//登录注册函数
void register_login(const int fd, const Value &results)
{
	int ret = 0;
	string username;
	string password;
	string response;

	printf("client connected!\n");
	forClient client("test1.db");
	client.CreateForm();
	username = results["username"].asString();
	password = results["password"].asString();

	switch (results["mode"].asInt())
	{
		case REGISTER:
			ret = client.table1_usrname_is_exist(username.c_str());
			if(ret)
			{
				//注册时用户名重复，无法继续注册，回应0给客户端
				response = FOUND_ERR;
				write(fd, response.c_str(), response.size());
				return ;
			}
			
			client.table1_insert_usr(username.c_str(), password.c_str());
			printf("REGISTER!\n");
			//有效注册，回应1给客户端
			response = SUCESS;
			write(fd, response.c_str(), response.size());
			break;
		case LOGIN:
			ret = client.table1_usrname_is_exist(username.c_str());
			printf("ret=%d\n",ret);
			if(ret)
			{
				char * usrpass = NULL;
				usrpass = client.table1_query_usr(username.c_str());
				printf("usrpass=%s\n",usrpass);
				if(strncmp(password.c_str(), usrpass, sizeof(usrpass)))
				{
					//密码错误，回应2给客户端
					response = ERRPASSWORD;
					write(fd, response.c_str(), response.size());
					free(usrpass);
					usrpass = NULL;
					return ;
				}
				//密码正确，回应3给客户端
				response = GREATPASSWORD;
				write(fd, response.c_str(), response.size());
				free(usrpass);
				usrpass = NULL;
				printf("LOGIN!\n");
			}
			else
			{
				printf("用户名输入错误，请重新输入\n");
				//用户名输入错误，回应1给客户端
				response = ERRUSERNAME;
				write(fd, response.c_str(), response.size());
			}

			break;
	}

	return ;
}

//用户管理函数
void user_manage(const int fd, const Value &results)
{
	int ret = 0;
	string response;
	Value root;
	FastWriter writer;

	printf("client connected!\n");
	forClient client("test1.db");
	client.CreateForm();

	struct emlpoyeeInfo usrInfo;
	usrInfo.id = results["id"].asUInt();
	usrInfo.name = results["name"].asString();
	usrInfo.gender = results["gender"].asString();
	usrInfo.phone = results["phone"].asString();

	switch (results["mode"].asInt())
	{
		case USER_INSERT:
			ret = client.table2_id_is_exist(usrInfo.id);
			if(ret)
			{
				//注册时用户名重复，无法继续注册，回应0给客户端
				response = FOUND_ERR;
				write(fd, response.c_str(), response.size());
				return ;
			}

			ret = client.table2_insert(usrInfo);
			if(ret != 0)
			{
				response = SQLITE_ERR;
			}
			else
			{
				//有效注册，回应1给客户端
				response = SUCESS;
			}

			printf("USER_INSERT!\n");
			write(fd, response.c_str(), response.size());
			break;
		case USER_QUERY:
			client.table2_queryAllMsg(root);
			response = writer.write(root);
			write(fd, response.c_str(), response.size());
			printf("USER_QUERY!\n");
			break;
		case USER_DELETE:
			ret = client.table2_delete(usrInfo.id);
			if(ret != 0)
			{
				response = SQLITE_ERR;
			}
			else
			{
				//有效注册，回应1给客户端
				response = SUCESS;
			}
			write(fd, response.c_str(), response.size());
			break;
		case USER_REVISE:
			ret = client.table2_update(usrInfo);
			if(ret != 0)
			{
				response = SQLITE_ERR;
			}
			else
			{
				//有效注册，回应1给客户端
				response = SUCESS;
			}
			
			write(fd, response.c_str(), response.size());	
			break;
		default:
			break;
	}

	return ;
}




//将文件I/O设置为非阻塞
int setnonblocking(int fd)
{
	int old_option = fcntl(fd,  F_GETFL);  // 获取文件描述符旧的状态标志
	int new_option = old_option | O_NONBLOCK; //设置非阻塞标志
	fcntl(fd, F_SETFL, new_option);  
	return old_option;  
}

#if 0
void epoll(void)
{
	int epoll_create(int size);//创建一个epoll实例，返回一个文件描述符，epoll维护一颗红黑书
	/*
	成功返回epfd,失败返回-1
	*/



    int epoll_ctl(int epfd, int op, int fd, struct epoll_event *event)//添加、修改、删除监视对象文件描述符
	/*
	 成功返回，失败返回-1
	1）epfd:epoll实例返回给用户的文件描述符
	2）op:用于指定监视对象的添加、删除、修改操作设置项
	op:
	1.EPOLL_CTL_ADD，添加
	2.EPOLL_CTL_DEL，删除
	3.EPOLL_CTL_MOD，修改
	3）fd:需要注册的监视对象的文件描述符
	4）监视对象的事件类型
	*/
	   struct epoll_event {
	       uint32_t     events;	 /* Epoll events */
	       epoll_data_t data;	 /* User data variable */
	   };
	/*
	epoll events对应的事件宏：
		1)EPOLLIN:read
		2)EPOLLOUT:write
		3)EPOLLET:边缘触发的方式得到事件通知
	*/
	   typedef union epoll_data {
	       void	   *ptr;
	       int	    fd;//常用是这个，文件描述符，返回后可查看
	       uint32_t     u32;
	       uint64_t     u64;
	   } epoll_data_t;


    int epoll_wait(int epfd, struct epoll_event *events,
		      int maxevents, int timeout);
	/*
	成功返回发生事件的文件描述符的数量，失败返回-1

	1)epfd:epoll实例的文件描述符
	2)events:保存发生事件的文件描述符的结构体的地址
	3）maxevents:第二个参数可以保存的最大事件数
	4）timeout:超时检测，-1时一直等待事件发生
	*/
	return;
} 

#endif
