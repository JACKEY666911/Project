#include<stdio.h>
#include<stdlib.h>
#include<string>
#include<iostream>
#include<string.h>
#include"sqlite.hpp"
#include"httpserver.hpp"
#include<ctime>

//构造函数
emplyInfo::emplyInfo(string fileName):fileName(fileName)
{
    db = NULL;
    //打开数据库
    int ret = sqlite3_open(fileName.c_str(), &db);
    if(ret != SQLITE_OK)
    {
        printf("Fail to open SQL, errmsg:%s\n", sqlite3_errmsg(db));
		sqlite3_close(db);
		exit(1);
    }

}

//析构函数
emplyInfo::~emplyInfo()
{
    printf("我emplyInfo析构了\n");
}

//创建三张表（固定的）
int emplyInfo::CreateForm()
{
    //创建表
    char sql[SQL_SIZE] = {0};
    memset(sql, 0, SQL_SIZE);
    snprintf(sql, SQL_SIZE, "create table if not exists table%d(name text, password text)", SQL_TABLE1);
    if(sqlite3_exec(db, sql, NULL,NULL, NULL) != SQLITE_OK)
	{
        exit(1);
        //ErrHandle(db);
	}

    printf("create table%d sucess!\n",SQL_TABLE1);
    
    memset(sql, 0, SQL_SIZE);
    snprintf(sql, SQL_SIZE, "create table if not exists table%d(id integer, name text, gender text, phone text, fingerID integer)", SQL_TABLE2);
    if(sqlite3_exec(db, sql, NULL,NULL, NULL) != SQLITE_OK)
	{
        ErrHandle(db);
	}

    printf("create table%d sucess!\n",SQL_TABLE2);
    
    memset(sql, 0, SQL_SIZE);
    snprintf(sql, SQL_SIZE, "create table if not exists table%d(year integer, month integer, day integer, id integer, name text, workdate text, gonedate text)", SQL_TABLE3);
    if(sqlite3_exec(db, sql, NULL,NULL, NULL) != SQLITE_OK)
	{
        ErrHandle(db);
	}
    printf("create table%d sucess!\n",SQL_TABLE3);

    return 0;
}

//查询所有员工信息数据
int emplyInfo::table2_queryAllMsg(Value &root)
{
    char sql[SQL_SIZE] = {0};
    memset(sql, 0, SQL_SIZE);//初始命令数组内容
	int row = 0;
	int cloumn = 0;
	char **result = NULL;
    Value obj;

    snprintf(sql, SQL_SIZE, "select * from table2");

	if(sqlite3_get_table(db, sql, &result, &row, &cloumn, NULL) != SQLITE_OK)
	{
		printf("get table fail!errormsg:%s\n", sqlite3_errmsg(db));
        root.append(SQLITE_ERR);
        sqlite3_close(db);
		return -1;
	}

    int counts = cloumn*(row + 1);
    int index = cloumn;
    if(row > 0)
    { 
        root.append(SUCESS);
        while(index < counts)
        {
            obj["id"]       = result[index++];
            obj["name"]     = result[index++];
            obj["gender"]   = result[index++];
            obj["phone"]    = result[index++];
            obj["fingerID"] = result[index++];
            root.append(obj);
            obj.clear();
        } 
    }
    else
    {
        printf("没有相关信息\n");
        root.append(FOUND_ERR);
        return -1;
    }

    sqlite3_free_table(result);
    return 0;
}

//有条件查询打卡记录表数据，并导出查询的数据
int emplyInfo::queryMsg(const int code, Value &root, int mode)
{
    //char buffer[80];
    time_t rawtime;
    time(&rawtime);
    struct tm *info;
    info = localtime(&rawtime);

    int month = info->tm_mon + 1;
    printf("%d\n", month);
    int day = info->tm_mday;

    char sql[SQL_SIZE] = {0};
	int row = 0;
	int cloumn = 0;
	char **result = NULL;
    if(code == CODE_1)
    {
        switch(mode)
        {
            case EACH_MONTH:
                snprintf(sql, SQL_SIZE, "select a.id, a.name, a.workdate, a.gonedate,b.fingerID from table3 a join table2 b using(id) where a.month = %d", month);
                break;
            case EACH_DAY:
                snprintf(sql, SQL_SIZE, "select a.id, a.name, a.workdate, a.gonedate,b.fingerID from table3 a join table2 b using(id) where a.month = %d and a.day = %d", 
                 month, day);
            default:
                break;
        }      
    }
    else if (code == CODE_2)
    {
        snprintf(sql, SQL_SIZE, "select a.id, a.name, a.workdate, a.gonedate,b.fingerID from table3 a join table2 b using(id) where a.month = %d and a.day = %d", 
                 month, day);
        cout <<  sql << endl;  
    }
    else
    {
	    root.append(FOUND_ERR);
        
        return -1;
    }
    
    if(sqlite3_get_table(db, sql, &result, &row, &cloumn, NULL) != SQLITE_OK)
	{
		printf("get table fail!errormsg:%s\n", sqlite3_errmsg(db));
        root.append(SQLITE_ERR);
        sqlite3_close(db);
		return -1;
	}
    cout << row << endl;
    Value obj;
    int counts = cloumn*(row + 1);
    int index = cloumn;
    if(row > 0)
    {
        root.append(SUCESS);
        while(index < counts)
		{
            obj["id"]   = result[index++];
            obj["name"] = result[index++];
            obj["workdate"] = result[index++];
            obj["gonedate"] = result[index++];
            obj["fingerID"] = result[index++];
            root.append(obj);
            obj.clear();
		}   
        cout << root.toStyledString() << endl;      
    }
    else
    {
        printf("没有相关信息，请重新选择日期\n");
        root.append(FOUND_ERR);
        return -1;
    }

    sqlite3_free_table(result);
	return 0;
}

//每月归零
int emplyInfo::table_delete(const int num)
{
    time_t rawtime;
    struct tm *info;
    info = localtime(&rawtime);
    char sql[SQL_SIZE];//同查询函数
    memset(sql, 0, SQL_SIZE);//初始命令数组内容
    snprintf(sql, SQL_SIZE, "delete from table%d", num);
    int ret = sqlite3_exec(db,sql,NULL,NULL,NULL);
    if(ret != SQLITE_OK)
    return ErrHandle(db);
    return 0;
}







//子类forClient的构造函数
forClient::forClient(string fileName):emplyInfo(fileName)
{

}
//子类forClient的析构函数
forClient::~forClient()
{

}

//向数据库中的table1表插入一项:用户名-密码
int forClient::table1_insert_usr(const char* username, const char* password)
{
	char sql[SQL_SIZE];
	memset(sql, 0, SQL_SIZE);//初始数组内容
	snprintf(sql, SQL_SIZE, "insert into table1 values('%s', '%s');", username, password);
    printf("%s\n",sql);
	int ret = sqlite3_exec(db,sql,NULL,NULL,NULL);
	return ret;
}

//返回一个用户名存在的条数,用于检查用户名是否已经存在
int forClient::table1_usrname_is_exist(const char* username)
{
    char sql[SQL_SIZE];//同查询函数
    int row,col,ret;
    char **res=NULL;
    memset(sql, 0, SQL_SIZE);//初始命令数组内容
    snprintf(sql, SQL_SIZE, "select name from table1 where name = '%s';",username);
    sqlite3_get_table(db,sql,&res,&row,&col,NULL);
    ret=row;
    sqlite3_free_table(res);
    return row;
}

//查询数据库中的client表中的name字段，返回其密码
char* forClient::table1_query_usr(const char* username)
{
    char sql[SQL_SIZE];
    int row,col;//分别表示查询结果的行数和列数，其中行数不包括列名
    char **res=NULL;//查询结果保存的地方，类似于string数组，结构是一维的，抽象来看是查询结果按行排列（包括列名）
    memset(sql, 0, SQL_SIZE);//初始命令数组内容
    snprintf(sql, SQL_SIZE, "select password from table1 where name = '%s';",username);
    sqlite3_get_table(db,sql,&res,&row,&col,NULL);
    
    if(row>0){
        int len = strlen(*(res+1));
        char* ret = (char*)malloc((len + 1)*sizeof(char));//分配堆空间，防止函数结束被释放
        if(!ret)
        {
            printf("malloc failed!\n");
            exit(1);
        }
        ret[len] = '\0';
        strcpy(ret,*(res+1));
        sqlite3_free_table(res);
        return ret;
    }
    else
        {
            printf("查询数据为空\n");
            sqlite3_free_table(res);
            return NULL;
        }
}

//向数据库中的table2表插入员工信息-id-name-gender-phone
int forClient::table2_insert(const struct emlpoyeeInfo &emlpoyeeInfo)
{
    int temp = -3;
    cout << "22222222222222" << endl;
	char sql[SQL_SIZE];
	memset(sql, 0, SQL_SIZE);
	snprintf(sql, SQL_SIZE, "insert into table2(id, name, gender, phone, fingerID)values(%d, '%s', '%s', '%s', %d);", 
            emlpoyeeInfo.id,emlpoyeeInfo.name.c_str(), emlpoyeeInfo.gender.c_str(), emlpoyeeInfo.phone.c_str(), temp);
	int ret = sqlite3_exec(db,sql,NULL,NULL,NULL);
    if(ret != SQLITE_OK)
    ErrHandle(db);
	return ret;
}

//返回一个id存在的条数,用于检查id是否已经存在
int forClient::table2_id_is_exist(const int &id)
{
    char sql[SQL_SIZE];//同查询函数
    int row,col,ret;
    char **res=NULL;
    memset(sql, 0, SQL_SIZE);//初始命令数组内容
    snprintf(sql, SQL_SIZE, "select id from table2 where id = %d;",id);
    if(sqlite3_get_table(db,sql,&res,&row,&col,NULL) != SQLITE_OK)
    ErrHandle(db);
    ret=row;
    sqlite3_free_table(res);
    return row;
}

//删除某个员工的信息
int forClient::table2_delete(const int &id)
{
    char sql[SQL_SIZE];//同查询函数
    memset(sql, 0, SQL_SIZE);//初始命令数组内容
    snprintf(sql, SQL_SIZE, "delete from table2 where id = %d;",id);
    int ret = sqlite3_exec(db,sql,NULL,NULL,NULL);
    if(ret != SQLITE_OK)
    ErrHandle(db);
    return 0;
}

//更改员工信息
int forClient::table2_update(const struct emlpoyeeInfo &emlpoyeeInfo)
{
    char sql[SQL_SIZE];//同查询函数
    memset(sql, 0, SQL_SIZE);//初始命令数组内容
    snprintf(sql, SQL_SIZE, "update table2 set name = '%s', gender = '%s', phone = '%s' where id = %d;",
                emlpoyeeInfo.name.c_str(), emlpoyeeInfo.gender.c_str(), emlpoyeeInfo.phone.c_str(), emlpoyeeInfo.id);
    int ret = sqlite3_exec(db,sql,NULL,NULL,NULL);
    if(ret != SQLITE_OK)
    ErrHandle(db);
    return 0;
}







//子类forDevice的构造函数
forDevice::forDevice(string fileName):emplyInfo(fileName)
{
    
}
//子类forDevice的析构函数
forDevice::~forDevice()
{
    
}
//在数据库插入打卡记录数据
int forDevice::Register(const struct insert &Mesg)
{
    int ret;
    int num = 3;
    string temp = "none"; 
    char buffer[80];
    time_t rawtime;
    struct tm *info;
    info = localtime(&rawtime);
    time(&rawtime);
    printf("%d年%d月%d日,日期:%s", info->tm_year + 1900,info->tm_mon + 1, info->tm_mday,ctime(&rawtime));
    strftime(buffer, 80, "%Y-%m-%d %H:%M:%S", info);
    printf("格式化的日期 : 时间 : %s\n", buffer );

    char sql[SQL_SIZE] = {0};
    
    memset(sql, 0, SQL_SIZE);

    bool workFlag = (info->tm_hour <= 14 ) && (info->tm_hour >= 4);
    bool goneFlag = (info->tm_hour >= 14 ) && (info->tm_hour <= 24);

    if(info->tm_mday == 1)
    {
        int flag = info->tm_mon;
        if(flag == 0)
        {
            flag = 12;
        }
        
        if(table3_month_is_exist(flag))
        {
            table_delete(num);
        }
    }
    ret = table3_id_is_exist(Mesg.id, info->tm_mday);
    cout << ret <<endl;
    if(ret == 0 && workFlag)
    {
        snprintf(sql, SQL_SIZE, "insert into table3(year, month, day, id, name, workdate, gonedate)values(%d, %d, %d, %d, '%s', '%s', '%s')", 
                info->tm_year + 1900,info->tm_mon + 1, info->tm_mday,
                Mesg.id, Mesg.name.c_str(), buffer, temp.c_str());
        cout << "打上班卡" << endl;
    }
    else if(ret == 0 && goneFlag)
    {
        snprintf(sql, SQL_SIZE, "insert into table3(year, month, day, id, name, workdate, gonedate)values(%d, %d, %d, %d, '%s', '%s', '%s')", 
                info->tm_year + 1900,info->tm_mon + 1, info->tm_mday,
                Mesg.id, Mesg.name.c_str(), temp.c_str(),buffer);
        cout << "打下班卡" << endl;
    }
    else
    {
        cout <<"table3_id_is_exist" << endl;
        //无效打卡
        return -2;
    }

    //插入数据
    if(sqlite3_exec(db, sql, NULL,NULL, NULL) != SQLITE_OK)
    return ErrHandle(db);

    return 0;
}
//更新员工指纹信息
int forDevice::table2_update(const int id, const int fingerID)
{
    char sql[SQL_SIZE];//同查询函数
    memset(sql, 0, SQL_SIZE);//初始命令数组内容
    snprintf(sql, SQL_SIZE, "update table2 set fingerID = %d where id = %d;", fingerID, id);
    int ret = sqlite3_exec(db,sql,NULL,NULL,NULL);
    if(ret != SQLITE_OK)
    ErrHandle(db);
    return 0;
}
//返回一个id和day存在的条数,用于检查是否已经打卡过了
int forDevice::table3_id_is_exist(const int &id, const int &day)
{
    char sql[SQL_SIZE];//同查询函数
    int row,col,ret;
    char **res=NULL;
    memset(sql, 0, SQL_SIZE);//初始命令数组内容
    snprintf(sql, SQL_SIZE, "select id from table3 where id = %d and day= %d ;",id, day);
    if(sqlite3_get_table(db,sql,&res,&row,&col,NULL) != SQLITE_OK)
    ErrHandle(db);
    ret=row;
    sqlite3_free_table(res);
    return row;
}
//查询当前月数是否到期

int forDevice::table3_month_is_exist(const int &month)
{
    char sql[SQL_SIZE];//同查询函数
    int row,col,ret;
    char **res=NULL;
    memset(sql, 0, SQL_SIZE);//初始命令数组内容
    snprintf(sql, SQL_SIZE, "select month from table3 where month = %d;", month);
    if(sqlite3_get_table(db,sql,&res,&row,&col,NULL) != SQLITE_OK)
    ErrHandle(db);
    ret=row;
    sqlite3_free_table(res);
    return row;
}


//出错判断
int ErrHandle(sqlite3 *db)
{
    printf("%s\n", sqlite3_errmsg(db));
	sqlite3_close(db);
	return -1;
}