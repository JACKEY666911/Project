#include"XthreadPool.hpp"
#include<iostream>
#include<string.h>
#include<stdlib.h>
#include<stdio.h>
#include<unistd.h>

ThreadPool::ThreadPool(int min, int max)
{
    do
    {
        //实例化任务队列
        taskQ = new TaskQueue;
        if(!taskQ)
        {
            cout << "new taskQ failed!" << endl;
            break;
        }
        //线程id创建，按照max创建
        threadIDs = new pthread_t[max];
        if(!threadIDs)
        {
            cout << "new threadIDs failed!" << endl;
            break;
        }

        memset(threadIDs, 0, sizeof(threadIDs));

        maxNum = max;
        busyNum = 0;
        aliveNum = 0;

        if(pthread_cond_init(&notEmpty, NULL) != 0 && pthread_mutex_init(&mutePool, NULL) != 0)
        {
            cout << "mutex or cond init failed!" << endl;
            break;
        }

        shutDown = false;
        goneThread = false;

        //创建管理者线程
        pthread_create(&managerThreadID, NULL, Manager, this);
        //创建工作线程
        for(int i = 0; i < min; ++i)
        {
            pthread_create(&threadIDs[i], NULL, Worker, this);
            if (pthread_detach(threadIDs[i]) != 0)
            {
                perror("detach worder fail\n");
                break;
            }
        }

        } while (0);

        if(threadIDs)
        delete []threadIDs;
        if(taskQ)
        delete taskQ;
}

ThreadPool::~ThreadPool()
{
    shutDown = true;

    for(int i = 0; i < aliveNum; ++i)
    {
        pthread_cond_signal(&notEmpty);
    }

    if(taskQ)
    delete taskQ;
    if(threadIDs)
    delete []threadIDs; 
    pthread_cond_destroy(&notEmpty);
    pthread_mutex_destroy(&mutePool);
}

int ThreadPool::PushJob(callback func, void *arg, int len)
{
    struct Task job;

    if(shutDown)
    {
        pthread_mutex_unlock(&mutePool);
        return -1;
    }

    job.arg = malloc(len);
    memcpy(job.arg, arg, len);
    job.function = func;
    taskQ->addTask(job);

    pthread_cond_signal(&notEmpty);

    return 0;
}

int ThreadPool::GetBusyNum()
{
    pthread_mutex_lock(&mutePool);
    int busyNum = this->busyNum;
    pthread_mutex_unlock(&mutePool);
    return busyNum;
}

int ThreadPool::GetAliveNum()
{
    pthread_mutex_lock(&mutePool);
    int aliveNum = this->aliveNum;
    pthread_mutex_unlock(&mutePool);
    return aliveNum;
}

void *ThreadPool::Worker(void *arg)
{
    //将this指针传进来，创建ThreadPool的类指针来接到对象的this指针
    ThreadPool * pool = (ThreadPool *)arg;
    while(true)
    {
        //对线程池的类成员上锁，每个线程对类成员的操作要保证原子性
        pthread_mutex_lock(&pool->mutePool);
        //需要循环判断任务队列是否有任务，
        while(pool->taskQ->getTaskNumber() == 0 && !pool->shutDown)
        {
            //判断是否要销毁线程
            if(pool->goneThread)
            {
                if(pool->aliveNum > pool->coreNum)
                {
                    pool->aliveNum--;
                    pthread_mutex_unlock(&pool->mutePool);
                    pool->ThreadExit();
                }
                else
                {
                    pool->goneThread = false;
                }
                
            }
            else
            {
                //阻塞当前线程，直到任务队列有新任务带来，试图抢锁
                pthread_cond_wait(&pool->notEmpty, &pool->mutePool);
            }
        }

        //判断线程池是否被销毁
        if(pool->shutDown)
        {
            pthread_mutex_unlock(&pool->mutePool);
            pool->ThreadExit();
        }

        //从任务队列取出任务来执行
        Task task = pool->taskQ->takeTask();
        pool->busyNum++;
        //对线程池解锁
        pthread_mutex_unlock(&pool->mutePool);

        //开始使用task中的函数指针和参数调用工作函数
        task.function(task.arg);
        //释放用Malloc申请的堆空间
        free(task.arg);
        task.arg = NULL;

        cout << "thread:" << pthread_self() << "is working..." << endl;

        //工作函数完成后，busyNum复原
        pthread_mutex_lock(&pool->mutePool);
        pool->busyNum--;
        pthread_mutex_unlock(&pool->mutePool);
    }

    return NULL;
}

void *ThreadPool::Manager(void *arg)
 {
    ThreadPool * pool = (ThreadPool*)arg;
    while(!pool->shutDown)
    {
        sleep(3);
        pthread_mutex_lock(&pool->mutePool);
        int aliveNum = pool->aliveNum;
        int busyNum = pool->busyNum;
        int queueSize = pool->taskQ->getTaskNumber();
        pthread_mutex_unlock(&pool->mutePool);

        //任务队列的任务个数大于等于当前存活线程，且当前存活线程小于最大线程数，
        //并大于核心线程数。可以选择增加线程数。
        bool flag = queueSize > aliveNum && aliveNum < pool->maxNum;
        if(flag)
        {
            pthread_mutex_lock(&pool->mutePool);
            int counter = 0;
            for(int i = 0; i < pool->maxNum && counter < 2 && flag; ++i)
            {
                if(pool->threadIDs[i] == 0)
                {
                    pthread_create(&pool->threadIDs[i], NULL, Worker, pool);
                    counter++;
                    pool->aliveNum++;
                }
            }
            pthread_mutex_unlock(&pool->mutePool);

        } 

        //销毁线程
        if(pool->aliveNum > pool->coreNum && pool->aliveNum > pool->busyNum)
        {
            pthread_mutex_lock(&pool->mutePool);
            pool->goneThread = true;
            pthread_mutex_unlock(&pool->mutePool);
        }
        else
        {
            pthread_mutex_lock(&pool->mutePool);
            pool->goneThread = false;
            pthread_mutex_unlock(&pool->mutePool);
        }
    }
 }

void ThreadPool::ThreadExit()
{
    pthread_t tid = pthread_self();
    for(int i = 0; i < maxNum; ++i)
    {
        if(threadIDs[i] == tid)
        {
            threadIDs[i] = 0;
            cout << tid << "exiting...." <<  endl;
            break;
        }
    }

    pthread_exit(NULL);
}