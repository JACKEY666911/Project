#ifndef XTHREADPOOL_HPP
#define XTHREADPOOL_HPP
#include"TaskQueue.hpp"

using namespace std;

class ThreadPool
{
    
public:
    //构造函数，用来初始化创建线程，线程的个数
    ThreadPool(int min, int max);
    //析构函数，用来销毁线程池
    ~ThreadPool();

    //面向用户的添加任务
    int PushJob(callback func, void *arg, int len);

    //获取忙线程个数
    int GetBusyNum();

    //获取存活的线程个数
    int GetAliveNum();

    //设置工作线程的任务函数,回调函数
    static void *Worker(void *arg);
    static void *Manager(void *arg);



    

private:
    //任务队列
    TaskQueue *taskQ;
    
    //创建管理者线程
    pthread_t managerThreadID;
    //需要用（动态）数组来描述工作队列
    pthread_t *threadIDs;
    //核心线程数
    int coreNum;
    //最大线程数量
    int maxNum;
    //空闲的线程个数
    int busyNum;
    //存活的线程个数
    int aliveNum;
    //设置条件变量，任务队列没任务时阻塞等待
    pthread_cond_t empty;   
    //设置互斥锁，配合条件变量
    pthread_mutex_t mutePool;
    //线程池销毁
    bool shutDown;

    //单个线程退出函数
    void ThreadExit();
};




#endif