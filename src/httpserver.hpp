#ifndef HTTPSERVER_HTTPSERVER_H
#define HTTPSERVER_HTTPSERVER_H

#include<event2/event.h>
#include<event2/http.h>
#include<event2/keyvalq_struct.h>
#include<event2/buffer.h>
#include<jsoncpp/json/json.h>
//#include"sqlite.hpp"

#define RECV_BUF_MAX_SIZE 1024*1024*1
#define REGISTER_LOGIN 14
#define REGISTER 15
#define LOGIN 16
#define USER_MANAGE 0xD
#define USER_INSERT 0xD1
#define USER_QUERY 0xD2
#define USER_DELETE 0xD3
#define USER_REVISE 0xD4

struct table_entry 
{
	const char *extension;
	const char *content_type;
}; 

class HttpServer
{
public:
    explicit HttpServer();
    ~HttpServer();

public:
    void start(const char* address, const int &port);

};

static const char *find_content_type(const char *path);
static void do_term(int sig, short events, void *arg);
void post_device_cb(struct evhttp_request *req, void *arg);
void post_client_cb(struct evhttp_request *req, void *arg);
static void document_cb(struct evhttp_request *req, void *arg);
void parse_post(struct evhttp_request *req, char *buf);

#endif