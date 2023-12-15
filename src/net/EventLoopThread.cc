#include "EventLoopThread.h"
#include "EventLoop.h"

EventLoopThread::EventLoopThread(const ThreadInitCallbaack &cb,
                                 const std::string &name)
    : loop_(nullptr)
    , exiting_(false)
    , thread_(std::bind(&EventLoopThread::ThreadFunc, this), name)
    , callback_(cb)
{
}

EventLoopThread::~EventLoopThread()
{
    exiting_ = true;
    if (loop_ != nullptr)
    {
        loop_->quit();
        thread_.join();
    }
}

EventLoop *EventLoopThread::startLoop()
{
    // 启动底层新线程
    thread_.start();

    // 这里考虑到主线程和新的子线程的同步问题，主线程如果直接返回的话
    // 子线程可能还没来的及在func_中创建EventLoop对象，可能会反会空对象
    // 所以主线程需要等待子线程创建完EventLoop对象在继续向下执行代码
    EventLoop *loop = nullptr;
    {
        std::unique_lock<std::mutex> lock(mutex_);
        cond_.wait(lock, [&]() -> bool
                   { return loop_ != nullptr; });
        loop = loop_;
    }

    return loop;
}

void EventLoopThread::ThreadFunc()
{
    EventLoop loop;
    if (callback_)
    {
        callback_(&loop);
    }

    {
        std::unique_lock<std::mutex> lock(mutex_);
        loop_ = &loop;
        cond_.notify_all();
    }

    
    loop.loop(); // 开始Reactor EventLoop
    
    //  退出事件循环
    std::unique_lock<std::mutex> lock(mutex_);
    loop_ = nullptr;
}