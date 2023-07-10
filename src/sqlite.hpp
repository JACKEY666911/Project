#ifndef __SQLITE_HPP__
#define __SQLITE_HPP__

#include<sqlite3.h>
#include<jsoncpp/json/json.h>
#include<string>

#define SQL_SIZE 1024
//用户名和密码表
#define SQL_TABLE1 0x1
//员工档案表
#define SQL_TABLE2 0x2
//打卡记录表
#define SQL_TABLE3 0x3

using namespace std;
using namespace Json;

struct time
{
    int year;
    int month;
    int day;
};

struct insert
{
    int id;
    string name;
    string date;
    string image;
};

//封装一个用于操作数据库的类
class emplyInfo
{
public:
    emplyInfo(string fileName);
    ~emplyInfo();
    int CreateForm();

protected:
    string fileName;
    sqlite3 *db;
};

class forClient:public emplyInfo
{
public:
    forClient(string fileName);

    int queryMsg(const struct time &date, Value &root);
    int queryMsg(Value &root);

    //向数据库中的client表插入一项:用户名-密码
    int sql_insert_usr(const char* usrname, const char* password);
    //返回一个用户名存在的条数,用于检查用户名是否已经存在
    int sql_usrname_is_exist(const char* usrname);
    //查询数据库中的client表中的usrname字段，返回其密码
    char* sql_query_usr(const char* usrname);
};

class forDevice:public emplyInfo
{
public:
    forDevice(string fileName);
    int Register(const struct insert &Mesg, const struct tm *info);

};

int ErrHandle(sqlite3 *db);

#endif















