#include "Channel.h"
#include "Logger.h"
#include "EventLoop.h"

#include <sys/epoll.h>

const int Channel::kNoneEvent = 0;
const int Channel::kReadEvent = EPOLLIN | EPOLLPRI;
const int Channel::kWriteEvent = EPOLLOUT;

Channel::Channel(EventLoop *loop, int fd)
    : loop_(loop), fd_(fd), events_(0), revents_(0), index_(-1), tied_(false)
{
}

Channel::~Channel()
{
}

// 什么时候调用呢？
void Channel::tie(const std::shared_ptr<void> &obj)
{
    tie_ = obj;
    tied_ = true;
}

// 通过channel所属的EventPoll中,删除当前channel
void Channel::Channel::remove()
{
    loop_->removeChannel(this);
}


// 该变channel所表示fd的events事件，update负责在poller里面更改fd相应的事件epoll_ctl
void Channel::update()
{
    // 通过channel所属的EventPoll,调用Poller的相应方法，注册fd的event
    loop_->updateChannel(this);
}

// fd等到poller通知后，处理事件
void Channel::handleEvent(Timestamp receiveTime)
{
    if (tied_)
    {
        std::shared_ptr<void> guard = tie_.lock();
        if (guard)
        {
            handleEventWithGuard(receiveTime);
        }
    }
    else
    {
        handleEventWithGuard(receiveTime);
    }
}

void Channel::handleEventWithGuard(Timestamp receiveTime)
{
    LOG_DEBUG("%s:%s:%d channel handleEvent revents:%d", \
            __FILE__, __FUNCTION__, __LINE__, revents_);
    if((revents_ & EPOLLHUP) && !(revents_ & EPOLLIN))
    {
        LOG_DEBUG("%s:%s:%d 断开连接\n", __FILE__, __FUNCTION__, __LINE__);
        if(closeCallback_)
        {
            closeCallback_();
        }
    }

    if(revents_ & EPOLLERR)
    {
        if(errorCallback_)
        {
            errorCallback_();
        }
    }

    if(revents_ & EPOLLIN)
    {
        if(readCallback_)
        {
            readCallback_(receiveTime);
        }
    }

    if(revents_ & EPOLLOUT)
    {
        if(writeCallback_)
        {
            writeCallback_();
        }
    }
}
