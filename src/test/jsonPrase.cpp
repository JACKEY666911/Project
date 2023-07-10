#include<stdio.h>
#include<stdlib.h>
#include<string>
#include<iostream>
#include<string.h>
//#include"sqlite.hpp"
#include<sqlite3.h>
#include<jsoncpp/json/json.h>
using namespace std;
using namespace Json;

#define SQL_SIZE 1024

/*
arg:用户传给回调函数的参数
ncolumn:查询后的列数，这一条记录有多少个字段 
vaule:查询结果的值的一维数组首地址
name:字段名
*/
// int  callback(void *arg, int n_column, char **column_value, char **column_name)
// {
//     int i;
    
//     printf( "记录包含 %d 个字段\n", n_column );

//     for( i = 0 ; i < n_column; i ++ )
//     {
//         printf( "字段名:%s |> 字段值:%s\n", column_name[i], column_value[i] );
//     }

//     printf( "------------------\n" );

//     return 0;

// }

// void searchName(sqlite3 *db, const string fileName, const char *name)
// {
//     //char *name = (char *)malloc(sizeof(char)*SQL_SIZE);

//     char sql[SQL_SIZE] = {0};
//     snprintf(sql, SQL_SIZE, "select id from %s where name = '%s'", 
//             fileName.c_str(), name);
//     printf("%s\n",sql);  
//     if(sqlite3_exec(db, sql, callback, NULL, NULL) != SQLITE_OK)
// 	{
// 		printf("select fail! errmsg:%s\n", sqlite3_errmsg(db));
// 		sqlite3_close(db);
// 		exit(1);
// 	}
// }



// int main()
// {
//     sqlite3 *db = NULL;
//     int ret = sqlite3_open("employee.db", &db);
//     if(ret != SQLITE_OK)
//     {
//         printf("Fail to open SQL, errmsg:%s\n", sqlite3_errmsg(db));
// 		sqlite3_close(db);
// 		exit(1);
//     }
//     string fileName = "employee";
//     int id = 3;
//     searchName(db,fileName, "何子豪");
//     //free(name);
//     return 0;
// }


