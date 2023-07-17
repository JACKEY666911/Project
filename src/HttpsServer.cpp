#include<openssl/ssl.h>
#include <openssl/err.h>
#include <event2/bufferevent_ssl.h>

#include<iostream>
#include<string>
#include<stdio.h>
#include<cstring>

#include<sys/types.h>
#include<sys/stat.h>

#include<signal.h>
#include<stdlib.h>
#include<dirent.h>

#include<unistd.h>
#include <fcntl.h>

#include"httpserver.hpp"
#include"sqlite.hpp"

const struct table_entry content_type_table[] = {
	{ "txt", "text/plain" },
	{ "c", "text/plain" },
	{ "h", "text/plain" },
	{ "json", "text/plain" },
	{ "html", "text/html" },
	{ "htm", "text/htm" },
	{ "css", "text/css" },
	{ "gif", "image/gif" },
	{ "jpg", "image/jpeg" },
	{ "jpeg", "image/jpeg" },
	{ "png", "image/png" },
	{ "pdf", "application/pdf" },
	{ "ps", "application/postscript" },
	{ NULL, NULL },
};



HttpServer::HttpServer() 
{

}

HttpServer::~HttpServer() 
{

}

void HttpServer::start(const char* address, const int &port) 
{
    //用于接受返回值
    int ret = 0;
    //创建libevent的上下文
    struct event_base *base = NULL;
    //创建http server事件
    struct evhttp *http = NULL;
    struct evhttp_bound_socket *handle = NULL;
    //终端进行ctrl+c发送停止信号
    struct event *term = NULL;

    //忽略管道信息
	if (signal(SIGPIPE, SIG_IGN) == SIG_ERR) 
	{
		ret = 1;
		goto err;
	}   

    //创建libevent的上下文
    base = event_base_new();
    if (!base) 
    {
	    fprintf(stderr, "Couldn't create an event_base: exiting\n");
	    return ;
	}

    //创建http server
    http = evhttp_new(base);
	if (!http) 
    {
		fprintf(stderr, "couldn't create evhttp. Exiting.\n");
		return ;
	}

	//设置默认的content-type
	evhttp_set_default_content_type(http, "text/html;charset=utf-8");
	
	printf("11111\n");
    //绑定地址和端口
    handle = evhttp_bind_socket_with_handle(http, address, port);
	ret = evhttp_bind_socket(http, address, port);
	if (ret == -1) 
    {
		fprintf(stderr, "couldn't bind to port %d. Exiting.\n", port);
		goto err;
	}

    //设置回调函数
	evhttp_set_cb(http, "/device", post_device_cb, NULL);
	evhttp_set_cb(http, "/client", post_client_cb, NULL);
    evhttp_set_gencb(http, document_cb, NULL);

	printf("2222\n");

    //添加信号事件（CTRL+C），并监视事件
	term = evsignal_new(base, SIGINT, do_term, base);//第四个参数是传参给回调函数要和第一个参数区分开
	if (!term)
		goto err;
	if (event_add(term, NULL))
		goto err;

    //设置超时
	evhttp_set_timeout(http, 30);
	//开启事件循环
    event_base_dispatch(base);
	printf("3333\n");
err:
	if (http)
		evhttp_free(http);
	if (term)
		event_free(term);
	if (base)
		event_base_free(base);
    return ;
}

//Try to guess a good content-type for 'path' 
static const char *find_content_type(const char *path)
{
	const char *last_period, *extension;
	const struct table_entry *ent;
	last_period = strrchr(path, '.');//从右往左获取“.”以及其后的内容，返回当前位置的指针
	if (!last_period || strchr(last_period, '/'))
		goto not_found; /* no exension */
	extension = last_period + 1;//指针指向下一个字符
	for (ent = &content_type_table[0]; ent->extension; ++ent) //循环遍历content_type_table中的extension,进行查找
	{
		if (!strcmp(ent->extension, extension))
			return ent->content_type;
	}

not_found:
	return "application/misc";
}

//终端被CTRL+C关闭时，回调此函数
static void do_term(int sig, short events, void *arg)
{
	struct event_base *base = (struct event_base *)arg;
	event_base_loopbreak(base);
	fprintf(stderr, "Got %i, Terminating\n", sig);
}

//针对设备端设置的回调函数
void post_device_cb(struct evhttp_request *req, void *arg)
{
	//printf("post111111\n");

	char buffer[RECV_BUF_MAX_SIZE];
	//获取请求信息
    parse_post(req, buffer);
	//Json解析
	Value results;
	Reader reader;
	int code = 23021;
	struct insert
	{
    	int id;
    	string name;
    	string date;
    	string image;
	};
	struct insert mesg ={.image ="111",};
	reader.parse(buffer, results);
	if(results.isObject())
	{
		if(results["code"].asInt() != code)
		{
			printf("非法请求\n");
			return ;
		}
		mesg.id 	= results["id"].asInt();
		mesg.name 	= results["name"].asString();
		mesg.date 	= results["date"].asString();
	}
	else
	{
		//printf("格式错误\n");
	}
	

	//回应请求
	//4.设置回应包
	printf("post333333333\n");
	struct evbuffer *retbuff = NULL;
	evhttp_add_header(evhttp_request_get_output_headers(req),
						"Content-Type", "text/plain;charset=utf-8,application/json");
	retbuff = evbuffer_new();
	evbuffer_add_printf(retbuff,"Receive post request,Thanks for the request!");
	evhttp_send_reply(req, 200,"OK",retbuff);
	evbuffer_free(retbuff);
    return ;
}

//针对Qt客户端设置的回调函数
void post_client_cb(struct evhttp_request *req, void *arg)
{
	char buffer[RECV_BUF_MAX_SIZE];
	//获取请求信息
    parse_post(req, buffer);

	//Json解析
	Value results;
	Reader reader;
	reader.parse(buffer, results);
	if(results["code"].asInt() != CODE_1)
	{
		printf("非法请求\n");
		return ;
	}

	if(results["flag"].asInt() == REGISTER_LOGIN)
	{	
		register_login(req, results);
	}
	else if(results["flag"].asInt() == USER_MANAGE)
	{
		user_manage(req, results);	
	}
	else
	{
		return ;
	}
	
	return ;
}

//用于处理GET请求，文件传输
static void document_cb(struct evhttp_request *req, void *arg)
{
	printf("12121212\n");
	struct evbuffer *evb = NULL;
	const char *uri = evhttp_request_get_uri(req);
	struct evhttp_uri *decoded = NULL;
	const char *path;
	size_t len;
	int fd = -1;
	struct stat st;
	const char *type;

	if (evhttp_request_get_command(req) != EVHTTP_REQ_GET)
	{
		return;
	}

	printf("Got a GET request for <%s>\n",  uri);

	decoded = evhttp_uri_parse(uri);
	if (!decoded) 
	{
		printf("It's not a good URI. Sending BADREQUEST\n");
		evhttp_send_error(req, HTTP_BADREQUEST, 0);
		return;
	}

	path = evhttp_uri_get_path(decoded);
	printf("Got a GET request for <%s>\n",  uri);
	if(!path)
	{
		path = "/";
	}
	if (stat(path, &st)<0)
	{
		goto err;
	}

	evb = evbuffer_new();
	if (S_ISDIR(st.st_mode))
	{
		DIR *d;
		struct dirent *ent;
				const char *trailing_slash = "";

		if (!strlen(path) || path[strlen(path)-1] != '/')
			trailing_slash = "/";
		goto err;
	}
	else
	{
		type = find_content_type(path);
		if ((fd = open(path, O_RDONLY)) < 0)
		{
			perror("open");
			goto err;
		}

		if (fstat(fd, &st)<0) {
			/* Make sure the length still matches, now that we
			 * opened the file :/ */
				perror("fstat");
				goto err;
			}
		evhttp_add_header(evhttp_request_get_output_headers(req),
		    "Content-Type", type);
		evbuffer_add_file(evb, fd, 0, st.st_size);
	}

	evhttp_send_reply(req, 200, "OK", evb);
	goto done;
err:
	evhttp_send_error(req, 404, "Document was not found");
	if (fd>=0)
		close(fd);
done:
if (decoded)
	evhttp_uri_free(decoded);
if (evb)
	evbuffer_free(evb);
	return;
}

//解析request请求，包括请求行，请求头，和把请求正文保存在buffer
void parse_post(struct evhttp_request *req, char *buffer)
{
	string cmdtype = "111";
    struct evkeyvalq *headers;
	struct evkeyval *header;
	struct evbuffer *buf;
	size_t post_lenth;

    const struct evhttp_uri* evhttp_uri;
    
    //1.获取request的请求信息
 
    //从request对象获取请求的uri信息和对象
    const char *uri = evhttp_request_get_uri(req);
    evhttp_uri = evhttp_request_get_evhttp_uri(req);
	
    //获取请求类型
    switch (evhttp_request_get_command(req)) 
    {
        case EVHTTP_REQ_GET: cmdtype = "GET"; break;
	    case EVHTTP_REQ_POST: cmdtype = "POST"; break;
        default: cmdtype = "unknown"; break;
    }

    //打印请求行信息
	cout << "Received a " << cmdtype <<"request for " << uri << endl << " Headers: " <<endl;
	if(evhttp_request_get_command(req) != EVHTTP_REQ_POST)
	{
		return ;
	}

    //2.获取消息报头
    headers = evhttp_request_get_input_headers(req);
    for (header = headers->tqh_first; header;
	    header = header->next.tqe_next) 
    {
		printf("  %s: %s\n", header->key, header->value);
	}

    //3.获取正文(GET为空，POST有表单信息  )
    buf = evhttp_request_get_input_buffer(req);
	post_lenth = evbuffer_get_length(buf);//获取数据长度
	if (post_lenth <= 0)
	{
		printf("post msg is empty!\n");
		return;
	}
	else
	{
    	while (post_lenth) 
    	{
			int n;
			//char cbuf[128];
			size_t copy_len = post_lenth > RECV_BUF_MAX_SIZE ? RECV_BUF_MAX_SIZE : post_lenth;
			n = evbuffer_remove(buf, buffer, copy_len);
			if (n > 0)
			{	
				fwrite(buffer, 1, n, stdout);
				//continue;
			}
			else
			{
				break;
			}
				
		}
	}
	
	return ;
}

//响应请求的函数
void response_post(struct evhttp_request *req, const char *response)
{
	printf("post333333333\n");
	struct evbuffer *retbuff = NULL;
	evhttp_add_header(evhttp_request_get_output_headers(req),
						"Content-Type", "text/plain;charset=utf-8,application/json");
	retbuff = evbuffer_new();

	evbuffer_add(retbuff, response, strlen(response));
	//evbuffer_add_printf(retbuff,response);
	evhttp_send_reply(req, 200, "OK", retbuff);
	evbuffer_free(retbuff);
}

//登录注册函数
void register_login(struct evhttp_request *req, const Value &results)
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
				response_post(req, response.c_str());
				return ;
			}
			
			client.table1_insert_usr(username.c_str(), password.c_str());
			printf("REGISTER!\n");
			//有效注册，回应1给客户端
			response = SUCESS;
			response_post(req, response.c_str());
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
					response_post(req, response.c_str());
					free(usrpass);
					usrpass = NULL;
					return ;
				}
				//密码正确，回应3给客户端
				response = GREATPASSWORD;
				response_post(req, response.c_str());
				free(usrpass);
				usrpass = NULL;
				printf("LOGIN!\n");
			}
			else
			{
				printf("用户名输入错误，请重新输入\n");
				//用户名输入错误，回应1给客户端
				response = ERRUSERNAME;
				response_post(req, response.c_str());
			}

			break;
	}

	return ;
}

//用户管理函数
void user_manage(struct evhttp_request *req, const Value &results)
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
				response_post(req, response.c_str());
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
			response_post(req, response.c_str());
			break;
		case USER_QUERY:
			client.table2_queryAllMsg(root);
			response = writer.write(root);
			response_post(req, response.c_str());
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
			response_post(req, response.c_str());
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
			
			response_post(req, response.c_str());	
			break;
		default:
			break;
	}

	return ;
}