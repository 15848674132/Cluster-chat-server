#include "EventLoop.h"
#include "Logger.h"
#include "Poller.h"
#include "Channel.h"

#include <unistd.h>
#include <sys/eventfd.h>

// 防止一个线程创建多个EventLoop thread_local
__thread EventLoop *t_loopThisThread = nullptr;

// 定义默认的poller的超时间
const int kPollerTimeMs = 10000;

//
int createEventfd()
{
    int evtfd = ::eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
    if (evtfd < 0)
    {
        LOG_FATAL("eventfd error:%d\n", errno);
    }

    return evtfd;
}

EventLoop::EventLoop()
    : looping_(false)
    , quit_(false)
    , callingPendingFunctors_(false)
    , threadId_(CurrentThread::tid())
    , poller_(Poller::newDefaultPoller(this))
    , wakeupFd_(createEventfd()), wakeupChannel_(new Channel(this, wakeupFd_))
    , currentActiveChannels_(nullptr)
{
    LOG_DEBUG("EventLoop Create %p in thread %d \n", this, threadId_);
    if (t_loopThisThread)
    {
        LOG_FATAL("Another EventLoop %p exists in this thread %d \n", t_loopThisThread, threadId_);
    }
    else
    {
        t_loopThisThread = this;
    }

    // 设置wakeupfd的事件类型以及事件发生后的回调函数
    wakeupChannel_->setReadCallback(std::bind(&EventLoop::handleRead, this));
    // 每一个eventloop都将监听wakeupchannel的EPOLLIN读事件了
    wakeupChannel_->enableReading();
}

EventLoop::~EventLoop()
{
    wakeupChannel_->disableAll();
    wakeupChannel_->remove();
    ::close(wakeupFd_);
    t_loopThisThread = nullptr;
}

// 开始事件循环
void EventLoop::loop()
{
    looping_ = true;
    quit_ = false;
    LOG_DEBUG("EventLoop %p start looping \n", this);

    while (!quit_)
    {
        activeChannels_.clear();
        // subLoop主要监听两类事件，clinetfd 和 wakeupfd
        pollReturnTime_ = poller_->poll(kPollerTimeMs, &activeChannels_);
        for (Channel *channel : activeChannels_)
        {
            // poller监听的channel发生事件了，然后上报给eventloop,eventloop通知channel处理相应的事件
            channel->handleEvent(pollReturnTime_);
        }

        doPendingFunctors();
    }
    LOG_DEBUG("EventLoop %p quit looping \n", this);
    looping_ = false;
}
// 退出事件循环
void EventLoop::quit()
{
    quit_ = true;

    // 因为有可能时其他线程退出其他线程的eventloop,而其他线程的运行状态是不确定的，可能在阻塞
    // 也有可能在处理events
    if (!isInLoopThread())
    {
        wakeup();
    }
}

// 在当前loop中执行cb
void EventLoop::runInLoop(Functor cb)
{
    if (isInLoopThread())
    {
        cb();
    }
    else
    {
        queueInLoop(std::move(cb));
    }
}
// 把cb放入到队列中，唤醒loop所在线程，执行cb
void EventLoop::queueInLoop(Functor cb)
{
    {
        std::unique_lock<std::mutex> lock(mutex_);
        pendingFunctors_.emplace_back(cb);
    }
    if (!isInLoopThread() || callingPendingFunctors_)
    {
        wakeup(); // 唤醒loop所在线程
    }
}
// 唤醒loop所在的线程
void EventLoop::wakeup()
{
    uint64_t one = 1;
    ssize_t n = ::write(wakeupFd_, &one, sizeof one);
    if(n != sizeof one)
    {
        LOG_ERROR("EventLoop::wakeup() write %lu bytes instead of 8 \n", n);
    }
}

// EventLoop的方法 =》 Poller
void EventLoop::updateChannel(Channel *channel)
{
    poller_->updateChannel(channel);
}
void EventLoop::removeChannel(Channel *channel)
{
    poller_->removeChannel(channel);
}
bool EventLoop::hasChannel(Channel *channel)
{
    return poller_->hasChannel(channel);
}

void EventLoop::doPendingFunctors() // 执行回调
{
    std::vector<Functor> functors;
    // 释放pendingFunctors_访问的锁，其他线程可能还会注册 手法
    callingPendingFunctors_ = true;
    {
        std::unique_lock<std::mutex> lock(mutex_);
        functors.swap(pendingFunctors_);
    }

    for(const  Functor& functor : functors)
    {
        functor(); // 执行当前线程的回调
    }

    callingPendingFunctors_ = false;
}
void EventLoop::handleRead() // wake up
{
    uint64_t one = 1;
    ssize_t n = ::read(wakeupFd_, &one, sizeof one);
    if (n != sizeof one)
    {
        LOG_ERROR("EventLoop::handleRead() read %ld bytes instead of 8\n", n);
    }
}