#ifndef HTTPSERVER_HTTPSERVER_H
#define HTTPSERVER_HTTPSERVER_H

#include<event2/event.h>
#include<event2/http.h>
#include<event2/keyvalq_struct.h>
#include<event2/buffer.h>
#include<jsoncpp/json/json.h>
//#include"sqlite.hpp"

#define RECV_BUF_MAX_SIZE 1024*1024*1
//code
#define CODE_1 23021

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

//flag
#define PC_CLOCK_QUERY 0xD
//mode
#define EACH_MONTH 0x5
#define EACH_DAY 0x6
//code
#define CODE_2 23020

//flag
#define DEVICE_MANAGE 0xF
//mode
#define FINGER_INSERT 0x1
#define USERMSG_QUERY 0x2

//flag
#define CLOCK_MANAGE 0xE
//mode
#define CLOCK_INSERT 0x1
#define CLOCK_QUERY 0x2

#define SQLITE_ERR "-1"
#define FOUND_ERR "0"
#define SUCESS "1"
#define ERRPASSWORD "2"
#define GREATPASSWORD "3"

#define ERRUSERNAME "1"

using namespace std;
using namespace Json;

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
void response_post(struct evhttp_request *req, const char *response);
void register_login(struct evhttp_request *req, const Value &results);
void user_manage(struct evhttp_request *req, const Value &results);

void device_query_update(struct evhttp_request *req, const Value &results);
void clock_manage(struct evhttp_request *req, const Value &results);

#endif