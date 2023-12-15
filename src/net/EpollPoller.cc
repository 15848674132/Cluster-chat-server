#include "EpollPoller.h"
#include "Logger.h"
#include "Channel.h"
#include <unistd.h>
#include <errno.h>
#include <string.h>

// channel未添加到poller中
const int kNew = -1; // channel的成员index_ = -1
// channel已添加到poller中
const int kAdded = 1;
// channel已从poller中删除
const int kDelete = 2;

EpollPoller::EpollPoller(EventLoop *loop)
    : Poller(loop), epollfd_(::epoll_create1(EPOLL_CLOEXEC))
    , events_(kInitEventListSize) // vector<struct epoll_event>
{
    if (epollfd_ < 0)
    {
        LOG_FATAL("epoll_create error:%d \n", errno);
    }
}

EpollPoller::~EpollPoller()
{
    ::close(epollfd_);
}

Timestamp EpollPoller::poll(int timeoutMs, ChannelList *activeChannels)
{
    LOG_DEBUG("%s:%s:%d  => fd total count:%ld\n", __FILE__, __FUNCTION__, __LINE__, channels_.size());
    int numEvents = epoll_wait(epollfd_, &*events_.begin(), static_cast<int>(events_.size()), timeoutMs);
    int saveErrno = errno;
    Timestamp now(Timestamp::now());
    
    if(numEvents > 0)
    {
        LOG_DEBUG("%d events happened \n", numEvents);
        fillActiveChannel(numEvents, activeChannels);
        if(numEvents == events_.size())
        {
            events_.resize(events_.size() * 2);
        }
    }
    else if(numEvents == 0)
    {
        LOG_DEBUG("func=%s timeout! \n", __FUNCTION__);
    }
    else 
    {
        if(saveErrno != EINTR)
        {
            errno = saveErrno;
            LOG_ERROR("EPollPoller::Poll() err!\n");
        }
    }
    return now;
}

void EpollPoller::updateChannel(Channel *channel)
{
    const int index = channel->index();
    LOG_DEBUG("%s:%s:%d => fd=%d events=%d index=%d \n", __FILE__, __FUNCTION__, __LINE__,  channel->fd(), channel->events(), index);
    if (index == kNew || index == kDelete)
    {
        int fd = channel->fd();
        if (index == kNew)
        {
            channels_[fd] = channel;
        }

        channel->set_index(kAdded);
        update(EPOLL_CTL_ADD, channel);
    }
    else // channel在poller中已经注册了
    {
        // 从epoll中删除channel维护的sockfd
        if (channel->isNoneEvent()) 
        {
            update(EPOLL_CTL_DEL, channel);
            channel->set_index(kDelete);
        }
        else // 修改已注册channel的关心事件
        {
            update(EPOLL_CTL_MOD, channel);
        }
    }
}

// 从Poller中删除channel
void EpollPoller::removeChannel(Channel *channel)
{
    int fd = channel->fd();
    channels_.erase(fd);
    int index = channel->index();
    if(index == kAdded) // kDelte已经做了下面的操作
    {
        update(EPOLL_CTL_DEL, channel);
    }
    channel->set_index(kNew);
}

// 填写活跃的连接
void EpollPoller::fillActiveChannel(int numEvent, ChannelList *activeChannels) const
{
    for(int i = 0; i < numEvent; ++i)
    {
        Channel *channel = static_cast<Channel*>(events_[i].data.ptr);
        channel->set_revents(events_[i].events);
        activeChannels->push_back(channel); // 就绪channel
    }
}

// 更新channel通道
void EpollPoller::update(int operation, Channel *channel)
{
    int fd = channel->fd();
    struct epoll_event ev;
    memset(&ev, 0, sizeof ev);
    ev.events = channel->events();
    ev.data.fd = fd;
    ev.data.ptr = channel; // 绑定fd携带的数据
    if (::epoll_ctl(epollfd_, operation, fd, &ev) < 0)
    {
        if(operation == EPOLL_CTL_DEL)
        {
            LOG_ERROR("epoll_ctl del error:%d\n", errno);
        }
        else
        {
            LOG_FATAL("epoll_ctl add/mod error:%d\n", errno);
        }
    }
}