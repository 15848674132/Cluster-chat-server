#ifndef EPOLL_POLLER_H
#define EPOLL_POLLER_H
#include "Poller.h"

#include <vector>
#include <sys/epoll.h>

class EventLoop;
class Channel; 

class EpollPoller : public Poller
{
public:
    EpollPoller(EventLoop *loop);
    ~EpollPoller() override;

    Timestamp poll(int timeoutMs, ChannelList *activeChannels) override;
    void updateChannel(Channel *channel) override;
    void removeChannel(Channel *channel) override;
private:
    static const int kInitEventListSize = 16;

    // 填写活跃的连接
    void fillActiveChannel(int numEvent, ChannelList *activeChannels) const;
    // 跟新channel通道
    void update(int operation, Channel *channel);

    using EventList = std::vector<epoll_event>;

    int epollfd_;
    EventList events_;
};



#endif