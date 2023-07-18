#ifndef TASKQUEUE_HPP
#define TASKQUEUE_HPP

#include<queue>
#include<pthread.h>
#include<jsoncpp/json/json.h>
using namespace Json;
using namespace std;

//using callback = void(*)(void *arg);

typedef void (*callback)(void *arg);

//创建任务结构体，内容为函数指针和参数
struct Task
{
    Task()
    {
        function = nullptr;
        arg = nullptr;
    }
    
    callback function;
    void *arg;

};


class TaskQueue
{
public:
    TaskQueue();
    ~TaskQueue();

    //添加任务
    void addTask(Task task);

    //取出任务
    Task takeTask();

    //用内联函数获取任务队列的当前任务数量
    inline int getTaskNumber()
    {
        return m_taskQ.size();
    }

private:
    queue<Task> m_taskQ;
    pthread_mutex_t mutex;

};

#endif