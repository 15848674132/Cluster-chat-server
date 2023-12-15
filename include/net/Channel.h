#ifndef CHANNEL_H
#define CHANNEL_H

#include <functional>
#include <memory>
#include "noncopyable.h"
#include "Timestamp.h"

/*
* channel 可以理解为对sockfd的封装
* 理清楚 EventLoop、channel、Poller之间的关系 Reactor模型上对应 demultiplex
* Channel 理解为通道，封装了sockfd和其感兴趣的event，如EPOLLIN、
* EPOLLOUT、EPOLLERR事件，还绑定了poller返回的具体事件
*/

class EventLoop;

class Channel : noncopyable
{
public:
    using EventCallback = std::function<void()>;
    using ReadEventCallback = std::function<void(Timestamp)>;

    Channel(EventLoop *loop, int fd);
    ~Channel();

    // fd得到poller通知以后，处理事件-> 通过调用相应就绪事件的回调函数
    void handleEvent(Timestamp receiveTime);
    void setReadCallback(ReadEventCallback cb) { readCallback_ = std::move(cb); }
    void setWriteCallback(EventCallback cb) { writeCallback_ = std::move(cb); }
    void setCloseCallback(EventCallback cb) { closeCallback_ = std::move(cb); }
    void setErrorCallback(EventCallback cb) { errorCallback_ = std::move(cb); }

    // 防止当channel被手动remove掉，channel还在执行回调操作
    void tie(const std::shared_ptr<void>&);
    
    int fd() const { return fd_; }
    int events() const { return events_; }
    void set_revents(int revt) { revents_ = revt; }


    // 修改fd关心的事件
    void enableReading() { events_ |= kReadEvent; update(); }
    void disableReading() { events_ &= ~kNoneEvent; update(); }
    void enalbeWriting() { events_ |= kWriteEvent; update(); }
    void disalbeWriting() { events_ &= ~kWriteEvent; update(); }
    void disableAll() { events_ = kNoneEvent; update(); }
    
    // 查看fd当前关心事件的状态
    bool isNoneEvent() const { return events_ == kNoneEvent; }
    bool isWriting() const { return events_ & kWriteEvent; }
    bool isReading() const { return events_ & kReadEvent; }

    // 
    int index() { return index_; }
    void set_index(int idx) { index_ = idx; }

    // one loop per thread
    EventLoop* ownerLoop() { return loop_; }
    void remove();
private:
    void update();
    void handleEventWithGuard(Timestamp receiveTime);
private:
    static const int kNoneEvent;
    static const int kReadEvent;
    static const int kWriteEvent; 

    EventLoop *loop_; // 事件循环
    const int fd_;    // Poller监听的事件
    int events_;      // 注册fd感兴趣的事件
    int revents_;    // 接受就绪事件
    int index_;

    std::weak_ptr<void> tie_;
    bool tied_;

    // 因为channel通道里面=能够获知fd最终发生的具体事件revents,所以它负责调用具体事件的回调函数
    // 都是TcpConnection设置的回调函数
    ReadEventCallback readCallback_;
    EventCallback writeCallback_;
    EventCallback closeCallback_;
    EventCallback errorCallback_;
};

#endif
 