#include<iostream>
#include<string>
#include<string.h>

#include<sys/types.h>
#include<sys/stat.h>

#include<event2/event.h>
#include<event2/http.h>
#include<event2/keyvalq_struct.h>
#include<event2/buffer.h>

#include<signal.h>
#include<stdlib.h>
#include<dirent.h>
#include<cstdio>


#include<unistd.h>
#include <fcntl.h>

#include<jsoncpp/json/json.h>

using namespace std;
using namespace Json;

static const struct table_entry 
{
	const char *extension;
	const char *content_type;
} content_type_table[] = {
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
/* Try to guess a good content-type for 'path' */
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

void post_request_cb(struct evhttp_request *req, void *arg)
{
	printf("post111111\n");
    string cmdtype = "111";
    struct evkeyvalq *headers;
	struct evkeyval *header;
	struct evbuffer *buf;
	size_t lenth;

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
    //2.获取消息报头
    headers = evhttp_request_get_input_headers(req);
    for (header = headers->tqh_first; header;
	    header = header->next.tqe_next) 
    {
		printf("  %s: %s\n", header->key, header->value);
	}

    //3.获取正文(GET为空，POST有表单信息  )
    buf = evhttp_request_get_input_buffer(req);
	lenth = evbuffer_get_length(buf);//获取数据长度
	if (lenth <= 0)
	{
		printf("post msg is empty!\n");
		return;
	}
	else
	{
    	puts("Input data: <<<");
    	while (lenth) 
    	{
			int n;
			char cbuf[128];
			n = evbuffer_remove(buf, cbuf, sizeof(cbuf));
			if (n > 0)
			{
				fwrite(cbuf, 1, n, stdout);
			}
			else
			{
				break;
			}
				
		}
		puts(">>>");
	}
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

static void document_cb(struct evhttp_request *req, void *arg)
{
	//printf("12121212\n");
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



int main()
{
	char *path = getenv("PWD");
	printf("%s\n",path);
    //用于接受返回值
    int ret = 0;
    //设置端口号
    int port = 8080;
    //设置IP地址
    const char *address = "192.168.174.128";
    //创建libevent的上下文
    struct event_base *base = NULL;
    //创建http server事件
    struct evhttp *http = NULL;
    //struct evhttp_bound_socket *handle = NULL;
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
	    return 1;
	}

    //创建http server
    http = evhttp_new(base);
	if (!http) 
    {
		fprintf(stderr, "couldn't create evhttp. Exiting.\n");
		return 1;
	}
	//设置默认的content-type
	evhttp_set_default_content_type(http, "text/html;charset=utf-8");
	
	printf("11111\n");
    //绑定地址和端口
	ret = evhttp_bind_socket(http, address, port);
	if (ret == -1) 
    {
		fprintf(stderr, "couldn't bind to port %d. Exiting.\n", port);
		goto err;
	}
    //设置回调函数
	evhttp_set_cb(http, "/post", post_request_cb, NULL);
	//evhttp_set_gencb(http, post_request_cb, NULL);
	evhttp_set_cb(http, "/get", document_cb, NULL);
    //evhttp_set_gencb(http, document_cb, NULL);
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
    return 0;
}