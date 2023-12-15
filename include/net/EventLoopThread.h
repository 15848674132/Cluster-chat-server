#ifndef EVENTLOOP_THREAD_H
#define EVENTLOOP_THREAD_H

#include <functional>
#include <mutex>
#include <condition_variable>
#include <string>

#include "noncopyable.h"
#include "Thread.h"

class EventLoop;

class EventLoopThread
{
public:
    using ThreadInitCallbaack = std::function<void(EventLoop*)>;

    EventLoopThread(const ThreadInitCallbaack& cb = ThreadInitCallbaack(),
        const std::string &name = std::string());
    ~EventLoopThread();

    EventLoop* startLoop();
private:
    void ThreadFunc();
private:
    EventLoop *loop_;
    bool exiting_;
    Thread thread_;
    std::mutex mutex_; // 实现主线程和子线程的线程同步通信
    std::condition_variable cond_;  //
    ThreadInitCallbaack callback_;
};






#endif