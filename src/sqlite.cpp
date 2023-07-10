#include<stdio.h>
#include<stdlib.h>
#include<string>
#include<iostream>
#include<string.h>
#include"sqlite.hpp"



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
        ErrHandle(db);
	}

    printf("create table%d  sucess!\n",SQL_TABLE1);
    
    memset(sql, 0, SQL_SIZE);
    snprintf(sql, SQL_SIZE, "create table if not exists table%d(id integer, name text, gender text, phone text)", SQL_TABLE2);
    if(sqlite3_exec(db, sql, NULL,NULL, NULL) != SQLITE_OK)
	{
        ErrHandle(db);
	}

    printf("create table%d  sucess!\n",SQL_TABLE2);
    
    memset(sql, 0, SQL_SIZE);
    snprintf(sql, SQL_SIZE, "create table if not exists table%d(year integer, month integer, day integer, id integer, name text, clockdate text)", SQL_TABLE3);
    if(sqlite3_exec(db, sql, NULL,NULL, NULL) != SQLITE_OK)
	{
        ErrHandle(db);
	}
    printf("create table%d  sucess!\n",SQL_TABLE3);

    return 0;
}

//子类forClient的构造函数
forClient::forClient(string fileName):emplyInfo(fileName)
{
    
}



//有条件查询数据，并导出查询的数据
int forClient::queryMsg(const struct time &date, Value &root)
{
    sqlite3 *db = NULL;
    char sql[SQL_SIZE] = {0};
	int row = 0;
	int cloumn = 0;
	char **result = NULL;
    bool dayFlag = date.day  <= 0 || date.day > 31;
    bool monthFlag = date.month  <= 0 || date.month > 12;

    if(dayFlag && !monthFlag)
    {
        snprintf(sql, SQL_SIZE, "select * from table3 where year = %d and month = %d", 
                    date.year, date.month);  
    }
    else if(monthFlag)
    {
        snprintf(sql, SQL_SIZE, "select * from table3 where year = %d", date.year);     
    }
    //else if (!dayFlag && monthFlag)
    //{
    //    root.append("ERROR");
    //}
    else
    {
	    snprintf(sql, SQL_SIZE, "select * from table3 where year = %d and month = %d and day = %d", 
                    date.year, date.month, date.day);
    }
    
	if(sqlite3_get_table(db, sql, &result, &row, &cloumn, NULL) != SQLITE_OK)
	{
		printf("get table fail!errormsg:%s\n", sqlite3_errmsg(db));
        root.append("SQLITE_ERR");
		return -1;
	}

    Value obj;
    int counts = cloumn*row;
    int index = cloumn;
    if(row > 1)
    {
        root.append("SUCESS");
        while(index < counts)
		{
            obj["id"]   = result[index++];
            obj["name"] = result[index++];
            obj["date"] = result[index++];
            root.append(obj);
            obj.clear();
		}
            
    }
    else
    {
        printf("没有相关信息，请重新选择日期\n");
        root.append("NOTFOUND_ERR");
        return -1;
    }

    sqlite3_free_table(result);
	return 0;
}

//查询所有数据
int forClient::queryMsg(Value &root)
{
    sqlite3 *db = NULL;
    char sql[SQL_SIZE] = {0};
	int row = 0;
	int cloumn = 0;
	char **result = NULL;
    Value obj;
	snprintf(sql, SQL_SIZE, "select * from table3");

	if(sqlite3_get_table(db, sql, &result, &row, &cloumn, NULL) != SQLITE_OK)
	{
		printf("get table fail!errormsg:%s\n", sqlite3_errmsg(db));
        root.append("SQLITE_ERR");
		return -1;
	}

    int counts = cloumn*row;
    int index = cloumn;
    if(row > 1)
    {
        root.append("SUCESS");
        while(index < counts)
		{
            obj["id"]   = result[index++];
            obj["name"] = result[index++];
            obj["date"] = result[index++];
            root.append(obj);
            obj.clear();
		}       
    }
    else
    {
        printf("没有相关信息\n");
        root.append("NOTFOUND_ERR");
        return -1;
    }

    sqlite3_free_table(result);
    return 0;
}




//向数据库中的client表插入一项:用户名-密码
int forClient::sql_insert_usr(const char* usrname, const char* password)
{
	char cmd[SQL_SIZE];
	memset(cmd, 0, SQL_SIZE);//初始数组内容
	snprintf(cmd, SQL_SIZE, "insert into table1 values('%s', '%s');", usrname, password);
    printf("%s\n",cmd);
	int ret = sqlite3_exec(db,cmd,NULL,NULL,NULL);
	return ret;
}

//返回一个用户名存在的条数,用于检查用户名是否已经存在
int forClient::sql_usrname_is_exist(const char* usrname)
{
    char cmd[SQL_SIZE];//同查询函数
    int row,col,ret;
    char **res=NULL;
    memset(cmd, 0, SQL_SIZE);//初始命令数组内容
    snprintf(cmd, SQL_SIZE, "select usrname from table1 where usrname like '%s';",usrname);
    sqlite3_get_table(db,cmd,&res,&row,&col,NULL);
    ret=row;
    sqlite3_free_table(res);
    return row;
}

//查询数据库中的client表中的usrname字段，返回其密码
char* forClient::sql_query_usr(const char* usrname)
{
    char cmd[SQL_SIZE];
    int row,col;//分别表示查询结果的行数和列数，其中行数不包括列名
    char **res=NULL;//查询结果保存的地方，类似于string数组，结构是一维的，抽象来看是查询结果按行排列（包括列名）
    memset(cmd, 0, SQL_SIZE);//初始命令数组内容
    snprintf(cmd, SQL_SIZE, "select password from table1 where usrname like '%s';",usrname);
    sqlite3_get_table(db,cmd,&res,&row,&col,NULL);
    
    if(row>1){
        int len=strlen(*(res+1));
        char* ret = (char*)malloc(len*sizeof(char));//分配堆空间，防止函数结束被释放
        if(!ret)
        {
            printf("malloc failed!\n");
            exit(1);
        }
        ret = *(res+1);
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


//子类forClient的构造函数
forDevice::forDevice(string fileName):emplyInfo(fileName)
{
    
}

//在数据库插入打卡记录数据
int forDevice::Register(const struct insert &Mesg, const struct tm *info)
{

    char sql[SQL_SIZE] = {0};
    //插入数据
    memset(sql, 0, SQL_SIZE);
    snprintf(sql, SQL_SIZE, "insert into table3 values(%d, %d, %d, %d, '%s', '%s')", 
                info->tm_year + 1900,info->tm_mon + 1, info->tm_yday,
                Mesg.id, Mesg.name.c_str(), Mesg.date.c_str());
    if(sqlite3_exec(db, sql, NULL,NULL, NULL) != SQLITE_OK)
	{
		printf("insert into fail! errmsg:%s\n", sqlite3_errmsg(db));
		sqlite3_close(db);
		exit(1);
	}

    return 0;
}

/*
char *searchName(const string fileName, const int id)
{
    sqlite3 *db = NULL;
    char sql[SQL_SIZE] = {0};
    snprintf(sql, SQL_SIZE, "select name from %s where id = %d", 
            fileName.c_str(), id);  
    if(sqlite3_exec(db, sql, callback,NULL, NULL) != SQLITE_OK)
	{
		printf("insert into fail! errmsg:%s\n", sqlite3_errmsg(db));
		sqlite3_close(db);
		exit(1);
	}
}


arg:用户传给回调函数的参数
ncolumn:查询后的列数，这一条记录有多少个字段 
vaule:查询结果的值的一维数组首地址
name:字段名

int  callback(void *arg, int n_column, char **column_value, char **column_name)
{
    int i;

    printf( "记录包含 %d 个字段\n", n_column );

    for( i = 0 ; i < n_column; i ++ )
    {
        printf( "字段名:%s |> 字段值:%s\n", column_name[i], column_value[i] );
    }

    printf( "------------------\n" );

    return 0;

}
*/
int ErrHandle(sqlite3 *db)
{
    printf("%s\n", sqlite3_errmsg(db));
	sqlite3_close(db);
	return -1;
}

