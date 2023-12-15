#include "EventLoopThreadPool.h"
#include "EventLoopThread.h"



EventLoopThreadPool::EventLoopThreadPool(EventLoop *loop, const std::string &nameArg)
    : baseLoop_(loop), name_(nameArg), started_(false), numThreads_(0), next_(0)
{}



void EventLoopThreadPool::start(const ThreadInitCallback &cb)
{
    started_ = true;
    for(int i = 0; i < numThreads_; ++i)
    {
        char buf[name_.size() + 32] = {0};
        snprintf(buf, sizeof buf, "%s%d", name_.c_str(), i);
        EventLoopThread *t = new EventLoopThread(cb, buf);
        threads_.push_back(std::unique_ptr<EventLoopThread>(t));
        // 创建线程并获取系统级线程tid && 为该线程创建EventLoop对象
        // 在EventLoopThread对象中就把线程和EventLoop进行绑定了
        // 从而实现one loop per thread
        loops_.push_back(t->startLoop()); 
    }

    if(numThreads_ == 0 && cb)
    {
        cb(baseLoop_);
    }
}

// 如果在在多线程中，baseLoop_会以轮询的方式分配channel给subloop
EventLoop *EventLoopThreadPool::getNextLoop()
{
    EventLoop *loop = baseLoop_;
    if(!loops_.empty())
    {
        loop = loops_[next_];
        ++next_;
        if(next_ >= loops_.size())
        {
            next_ = 0;
        }
    }

    return loop;
}

std::vector<EventLoop *>EventLoopThreadPool::getAllLoops()
{
    if(loops_.empty())
    {
        return std::vector<EventLoop*>(1, baseLoop_);
    }
    else
    {
        return loops_;
    }
}