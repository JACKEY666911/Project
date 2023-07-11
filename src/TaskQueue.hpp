#include<queue>

using namespace std;
//using callback = void(*)(void *arg);

typedef void (*callback)(void *arg);

//创建任务结构体，内容为函数指针和参数
typedef struct Task
{
    Task()
    {
        function = nullptr;
        arg = nullptr;
    }

    Task(callback function, void *arg)
    {
        this->arg = arg;
        this->function = function;
    }

    callback function;
    void *arg;

}Task;


class TaskQueue
{
public:
    TaskQueue();
    ~TaskQueue();

    //添加任务
    void addTask(Task task);
    //取出任务
    Task takeTask();
    //用内联函数获取任务队列的当前任务
    inline int getTaskNumber()
    {
        return m_task.size();
    }

private:
    queue<int> m_task;

};