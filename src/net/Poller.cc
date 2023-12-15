#include "Poller.h"

#include "Channel.h"

Poller::Poller(EventLoop *loop) 
    : ownerLoop_(loop)
{}

// 判断channel是否在当前Poller当中
bool Poller::hasChannel(Channel *channel) const
{
    auto iter = channels_.find(channel->fd());
    return iter != channels_.end() && channel == iter->second;
}

// EventLoop可以通过该接口获取默认的IO复用的具体实现
static Poller *newDefault(EventLoop *loop);