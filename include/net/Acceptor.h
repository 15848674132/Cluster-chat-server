#ifndef ACCEPTOR_H
#define ACCEPTOR_H


#include <functional>

#include "Socket.h"
#include "noncopyable.h"
#include "Channel.h"


class EventLoop;

class Acceptor : noncopyable
{
public:
    using NewConnectionCallback = std::function<void(int sockfd, const InetAddress&)>;
    Acceptor(EventLoop *loop, const InetAddress& listenAddr, bool reuseport);
    ~Acceptor();

    void setNewConnectCallback(const NewConnectionCallback& cb)
    {
        newConnectionCallback_ = cb;
    }

    bool listening() const { return listening_; }

    void listen();
private:

    void handRead();

    EventLoop *loop_; //mianLoop
    Socket acceptSocket_;
    Channel acceptChannel_;
    NewConnectionCallback newConnectionCallback_;
    bool listening_;
};




#endif