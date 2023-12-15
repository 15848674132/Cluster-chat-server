#ifndef SOCKET_H
#define SOCKET_H

#include "noncopyable.h"

class InetAddress;

class Socket : noncopyable
{
public:
    explicit Socket(int sockfd)
        : sockfd_(sockfd)
    {}

    ~Socket();

    int fd() const { return sockfd_; }
    void bindAddress(const InetAddress&);
    void listen();
    int accept(InetAddress *peeraddr);

    void shutDownWrite();
    void setTcpNoDelay(bool on);
    void setResueAddr(bool on);
    void setReusePort(bool on);
    void setKeepAlive(bool on);
private:
    int sockfd_;
};

#endif