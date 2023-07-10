#include<iostream>
#include<ctime>

using namespace std;
int main()
{
   time_t rawtime;
   struct tm *info;
   char buffer[80];
 
   time( &rawtime );
 
   info = localtime( &rawtime );
   printf("%d年%d月%d日，日期：%s", info->tm_year + 1900,info->tm_mon + 1, info->tm_mday,asctime(info));
 
   return(0);

}


