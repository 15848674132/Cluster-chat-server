#include "Thread.h"
#include"CurrentThread.h"

#include <semaphore.h>

std::atomic_int Thread::numCreate_(0);

Thread::Thread(ThreadFunc func, const std::string &name)
    : started_(false), joined_(false), tid_(0), func_(std::move(func)), thName_(name)
{
    setDefaultName();
}

Thread::~Thread()
{
    if(thread_ && !joined_)
    {
        thread_->detach();
    }
}

void Thread::start()
{
    started_ = true;
    // 开启新线程
    sem_t sem;
    sem_init(&sem, false, 0);
    // 这代码存在竞态条件，是线程不安全的 
    thread_ = std::make_shared<std::thread>(std::thread([&](){
        tid_ = CurrentThread::tid();
        sem_post(&sem); 
        func_();
    }));
    
    sem_wait(&sem); // 主线程阻塞，获取tid_
}

void Thread::join()
{
    joined_ = true;
    thread_->join();
}

void Thread::setDefaultName()
{
    int num = ++numCreate_;
    if(thName_.empty())
    {
        char buf[32] = {0};
        snprintf(buf, sizeof buf, "Thread%d", num);
        thName_ = buf;
    }
}