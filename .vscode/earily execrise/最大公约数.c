#include<stdio.h>
int main()
{
    int a, b;
    scanf("%d %d",&a,&b);
    printf("a= %d b = %d\n", a, b);
    int i = 2;
    int temp = 1;
    for(; i <= 10; ++i)
    {
        if(a % i == 0 && b % i == 0)
        {
            a /= i;
            b /= i;  
            temp *= i;
            i = 1;     
        }
    }
   
    printf("最大公约为:%d\n",temp);
    return 0;
}