#ifndef __SQLITE_HPP__
#define __SQLITE_HPP__

#include<sqlite3.h>
#include<jsoncpp/json/json.h>
#include<string>

#define SQL_SIZE 512
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

struct emlpoyeeInfo
{
    unsigned int id;
    string name;
    string gender;
    string phone;
    int fingerID;
};

struct insert
{
    int id;
    string name;
};

//封装一个用于操作数据库的类
class emplyInfo
{
public:
    emplyInfo(string fileName);
    ~emplyInfo();
    //创建三张表（固定的）
    int CreateForm();
    //查询所有员工信息数据
    //int table2_queryAllMsg(const int code, Value &root);
    int table2_queryAllMsg(Value &root);
    //有条件查询打卡记录表数据，并导出查询的数据
    int queryMsg(const int code, Value &root, int mode);
    //每月归零，清空table3
    int table_delete(const int num);
protected:
    string fileName;
    sqlite3 *db;

};

//子类forClient
class forClient:public emplyInfo
{
public:
    forClient(string fileName);
    ~forClient();

    //向数据库中的client表插入一项:用户名-密码
    int table1_insert_usr(const char* usrname, const char* password);
    //返回一个用户名存在的条数,用于检查用户名是否已经存在
    int table1_usrname_is_exist(const char* usrname);
    //查询数据库中的client表中的usrname字段，返回其密码
    char* table1_query_usr(const char* usrname);
    //向数据库中的table2表插入员工信息-id-name-gender-phone
    int table2_insert(const struct emlpoyeeInfo &emlpoyeeInfo);
    //返回一个id存在的条数,用于检查id是否已经存在
    int table2_id_is_exist(const int &id);
    //删除某个员工的信息
    int table2_delete(const int &id);
    //更改员工的联系方式
    int table2_update(const struct emlpoyeeInfo &emlpoyeeInfo); 
};

//子类forDevice
class forDevice:public emplyInfo
{
public:
    forDevice(string fileName);
    ~forDevice();
    //在数据库插入打卡记录数据
    int Register(const struct insert &Mesg);
    //更新员工指纹信息
    int table2_update(const int id, const int fingerID);
    //返回一个id和day存在的条数,用于检查是否已经打卡过了
    int table3_id_is_exist(const int &id, const int &day);
    //查询当前月数是否到期
    int table3_month_is_exist(const int &month);
};

int ErrHandle(sqlite3 *db);

#endif