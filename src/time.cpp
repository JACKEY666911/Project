#include<iostream>
#include<ctime>
#include <stdio.h>
#include<unistd.h>
using namespace std;

/*
struct tm {
   int tm_sec;         // 秒，范围从 0 到 59                
   int tm_min;         //分，范围从 0 到 59               
   int tm_hour;        // 小时，范围从 0 到 23               
   int tm_mday;        // 一月中的第几天，范围从 1 到 31                  
   int tm_mon;         // 月份，范围从 0 到 11                
   int tm_year;        // 自 1900 起的年数                
   int tm_wday;        // 一周中的第几天，范围从 0 到 6               
   int tm_yday;        // 一年中的第几天，范围从 0 到 365                    
   int tm_isdst;       // 夏令时                      
};
*/

int main()
{
   //time_t	从纪元起的时间类型
   //tm	日历时间类型

   time_t rawtime;
   struct tm *info;
//time_t time (time_t *__timer):返回自纪元起计的系统当前时间
//struct tm *localtime (const time_t *__timer):将time_t转换成本地时间

//char *ctime (const time_t *__timer):转换 time_t 对象为文本表示
//char *asctime (const struct tm *__tp):转换 tm 对象为文本表示

/*size_t strftime (char *__restrict __s, size_t __maxsize,
			const char *__restrict __format,
			const struct tm *__restrict __tp)

   转换 tm 对象到自定义的文本表示
*/
   char buffer[80];

   time( &rawtime );

   info = localtime(&rawtime);
   printf("%d年%d月%d日，日期：%s", info->tm_year + 1900,info->tm_mon + 1, info->tm_mday,ctime(&rawtime));


 
   strftime(buffer, 80, "%Y-%m-%d %H:%M:%S", info);
   printf("格式化的日期 : 时间 : %s\n", buffer );

 //double difftime(std::time_t time_end, std::time_t time_beg):计算时间之间的差
   time_t start_t, end_t;
   double diff_t;
 
   printf("程序启动...\n");
   time(&start_t);
 
   printf("休眠 5 秒...\n");
   sleep(5);
 
   time(&end_t);
   diff_t = difftime(end_t, start_t);
 
   printf("执行时间 = %f\n", diff_t);
   printf("程序退出...\n");

 
   return(0);

}


