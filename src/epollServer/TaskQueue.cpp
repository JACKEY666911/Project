#include"TaskQueue.hpp"
#include "epollserver.hpp"

//构造函数初始化互斥锁
TaskQueue::TaskQueue()
{
    pthread_mutex_init(&mutex, NULL);
}
//析构函数销毁互斥锁
TaskQueue::~TaskQueue()
{
    pthread_mutex_destroy(&mutex);
}

//向任务队列添加任务
void TaskQueue::addTask(Task task)
{
    pthread_mutex_lock(&mutex);
    m_taskQ.push(task);
    pthread_mutex_unlock(&mutex);
}


//取出任务队列中的任务
Task TaskQueue::takeTask()
{
    Task temp;
    pthread_mutex_lock(&mutex);
    if(!m_taskQ.empty())
    {
        temp = m_taskQ.front();
        m_taskQ.pop();
    }
    pthread_mutex_unlock(&mutex);
    return temp;
}