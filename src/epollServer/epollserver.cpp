#include<stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <arpa/inet.h>
#include<string.h>
#include<unistd.h>

#include <fcntl.h>
#include<errno.h>

#include<iostream>

#include"epollserver.hpp"
#include"XthreadPool.hpp"
#include"TaskQueue.hpp"

eopllServer::eopllServer()
{
	tcp_sockfd = 0;
	epfd = 0;
}

eopllServer::~eopllServer()
{
	
}

void eopllServer::listen_init(const char *address, const int port)
{
	int option = 1;
	tcp_sockfd = socket(AF_INET, SOCK_STREAM, 0);
	setsockopt(tcp_sockfd, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option));
	ERRHAND(tcp_sockfd,"sockect");

	struct sockaddr_in serverAddr;
	socklen_t serLength = sizeof(serverAddr);
	memset(&serverAddr, 0, serLength);
	
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(port); 
	//serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    inet_aton(address, &serverAddr.sin_addr);
	/*
	函数原型为char *inet aton(const char *string, struct in_addr *adr);
	作用是将网络字节序转换成字符串类型
	*/
	int ret = 0;
	ret = bind(tcp_sockfd, (struct sockaddr*)&serverAddr, serLength);
	ERRHAND(ret, "bind");
	ret = listen(tcp_sockfd, 10);
	ERRHAND(ret, "listen");
}

void eopllServer::accpet_start()//ThreadPool *pool
{
	int ret;
	int epoll_cnt;
	int accept_sock;
	struct epoll_event ev;
	struct epoll_event epevents[EVENT_SIZE] = {0};
	int event_size = sizeof(epevents);
	ev.events = EPOLLIN;
	ev.data.fd = tcp_sockfd;

	struct sockaddr_in clientAddr;
	socklen_t cliLength = 0;
	memset(&clientAddr, 0, cliLength);

	epfd = epoll_create(1);
	if(epfd == -1)
	{
		perror("epoll_create");
		exit(1);
	}

    ret = epoll_ctl(epfd, EPOLL_CTL_ADD , tcp_sockfd, &ev);//添加服务器端套接字
	ERRHAND(ret, "epoll_ctl");
	
	ThreadPool *pool = new ThreadPool(3, 8);

	while(1)//迭代服务器，处理一个客户端的数据传递后，继续受理后续的客户端
	{
		printf("88888888888888888\n");
		epoll_cnt = epoll_wait(epfd, epevents, event_size, -1);
		printf("5555555555555\n");
		printf("epoll_cnt:%d\n",epoll_cnt);
		ERRHAND(epoll_cnt, "epoll_wait");

		for(int i = 0; i < epoll_cnt; ++i)
		{
			if(epevents[i].data.fd == tcp_sockfd)
			{
				accept_sock = accept(tcp_sockfd, (struct sockaddr *)&clientAddr, &cliLength);
				ERRHAND(ret, "accept");
				char *str_clientAddr = inet_ntoa(clientAddr.sin_addr);
				int clientPort = ntohs(clientAddr.sin_port);
				printf("有人连接服务器,IP地址为%s,端口号为%d\n",str_clientAddr,clientPort);

				ev.events = EPOLLIN ;//| EPOLLET;//;//将epoll事件设置为边沿触发模式
				ev.data.fd = accept_sock;

				ret = epoll_ctl(epfd, EPOLL_CTL_ADD, accept_sock, &ev);
				ERRHAND_CON(ret, "epoll_ctl");
				printf("connected client: %d\n", accept_sock);
			}
			else
			{
				int clientfd = epevents[i].data.fd;
				//setnonblocking(clientfd);
				Value results;
				getJsonMesg(clientfd, epfd, results);
				FastWriter writer;
				cout << writer.write(results);
				Handle(clientfd, &results, pool);
				sleep(20);
				
			}		
		}
	}
	close(tcp_sockfd);
	close(epfd);
	delete pool;
	return ;
}

void Handle(int fd, Value *results, ThreadPool *pool)
{
	struct arg *Qarg = new struct arg;
	Qarg->results = results;
	Qarg->fd = fd;
	FastWriter writer;
	cout << "Handle" << writer.write(Qarg->results) << " fd: " << Qarg->fd;
	cout<< "Handle" << (*results)["password"].asString() << endl;
	if((*results)["code"].asInt() != CODE_1)
	{
		cout << "22222非法请求,退出！" << endl;
		return;
	}

	switch ((*results)["code"].asInt())
	{
		case CODE_1:
			pool->PushJob(forQtClient, Qarg, sizeof(struct arg));
			printf("coming\n");
			break;
		case CODE_2:
			pool->PushJob(forQtDevice, Qarg, sizeof(struct arg));
			break;				
		default:
			break;
	}
	
	delete Qarg;
	printf("44444444444444\n");
	return ;
}

void getJsonMesg(const int fd, const int epfd, Value &results)
{
	setnonblocking(fd);
	Reader reader;
	string Mesg;
	char mesg[MESG_SIZE] = {0};
	//char *mesg = new char[MESG_SIZE];
	memset(mesg, 0, MESG_SIZE);
	int str_len = 0;
	/*在epoll边缘触发的条件下，并将read事件对应的文件描述符设置为非阻塞I/O模式，
		这时用户设置的用户空间缓冲区可以不用太大，也可以减少epoll的检测次数，得用while循环把recv缓冲区数据读完
	*/
	while(1)
	{
		memset(mesg, 0, sizeof(mesg));
		str_len = recv(fd, mesg, MESG_SIZE, 0);	
		if(str_len < 0)
		{
			if(errno == EAGAIN )//|| errno == EWOULDBLOCK
			{
				
				printf("3333333333333333333\n");
				continue; //资源暂不可用， 在尝试一次
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
			printf("有客户端断开连接,disconnected client:%d\n",fd);
			close(fd);
			return;
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
	reader.parse(Mesg, results);
	cout << "ingetjson " << results["code"].asInt() << endl;
	//printf("22222222222222222\n");
	return ;
}


void forQtDevice(void *arg)
{
	struct arg *Qarg = (struct arg *)arg;
	const Value results = Qarg->results;
	const int fd = Qarg->fd;
	if(results["code"].asInt() != CODE_2)
	{
		printf("非法请求\n");
		return ;
	}

	return ;
}


void forQtClient(void *arg)
{
	struct arg *Qarg = (struct arg *)arg;
	const int fd = Qarg->fd;
	cout << "forQtClient" << fd << endl;
	cout << "forQtClient" << endl;
	Value *results = Qarg->results;
	cout << "forQtClient" << (*results).toStyledString();
	
	if((*results)["code"].asInt() != CODE_1)
	{
		printf("非法请求\n");
		return ;
	}

	if((*results)["flag"].asInt() == REGISTER_LOGIN)
	{	
		register_login(fd, (*results));
	}
	else if((*results)["flag"].asInt() == USER_MANAGE)
	{
		user_manage(fd,(*results));	
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
	string dbname = "test7.db";
	printf("client connected!\n");
	forClient client(dbname);
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
	int old_option = fcntl(fd, F_GETFL);  // 获取文件描述符旧的状态标志
	int new_option = old_option | O_NONBLOCK; //设置非阻塞标志
	fcntl(fd, F_SETFL, new_option);  
	return old_option;  
}

