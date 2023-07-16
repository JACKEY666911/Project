#include"XthreadPool.hpp"
#include"TaskQueue.hpp"
#include<stdio.h>
#include<unistd.h>

void testFun(void* arg)
{
    printf("i = %d\n", *(int *)arg);
    sleep(1);
}


int main()
{
    ThreadPool *pool = new ThreadPool(3, 10);

    printf("线程池初始化成功\n");

    int i = 0;
    for (i = 0; i < 30; ++i) 
    {
        int * num = new int(i+100);
        pool->PushJob(testFun, num, sizeof(int));
    }

    
    
    printf("aliveNum：%d\n",pool->GetAliveNum());
    sleep(20);
    delete pool;
    return 0;
}
