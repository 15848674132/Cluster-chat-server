#include "Acceptor.h"
#include "Logger.h"
#include "InetAddress.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>

static int createNonblocking()
{
    int sockfd = ::socket(AF_INET, SOCK_STREAM | SOCK_CLOEXEC | SOCK_NONBLOCK, IPPROTO_TCP);
    if (sockfd < 0)
    {
        LOG_FATAL("%s:%s:%d listen socket create err:%d \n", __FILE__, __FUNCTION__, __LINE__, errno);
    }

    return sockfd;
}

Acceptor::Acceptor(EventLoop *loop, const InetAddress &listenAddr, bool reuseport)
    : loop_(loop)
    , acceptSocket_(createNonblocking()) //创建非阻塞的listenfd，并且封装为Socket对象
    , acceptChannel_(loop, acceptSocket_.fd()) // 初始化acceptChannel_
    , listening_(false)
{
    acceptSocket_.setResueAddr(true); 
    acceptSocket_.setReusePort(true); 
    acceptSocket_.bindAddress(listenAddr); 
    // 设置acceptChannel_的读事件就绪的回调函数
    acceptChannel_.setReadCallback(std::bind(&Acceptor::handRead, this));
}

Acceptor::~Acceptor()
{
    acceptChannel_.disableAll();
    acceptChannel_.remove();
}

void Acceptor::listen()
{
    LOG_INFO("%s:%s:%d listenfd:%d start listen", __FILE__, __FUNCTION__, __LINE__, acceptSocket_.fd());
    listening_ = true;
    acceptSocket_.listen();
    // 这里当时忘记将acceptChannel添加到poller中了
    acceptChannel_.enableReading();
}

void Acceptor::handRead()
{
    InetAddress peerAddr;
    int connfd = acceptSocket_.accept(&peerAddr);
    if (connfd > 0)
    {
        if (newConnectionCallback_)
        {
            newConnectionCallback_(connfd, peerAddr); // 轮询找到subLoop，并唤醒，为其添加新的channel
        }
        else
        {
            ::close(connfd);
        }
    }
    else
    {
        LOG_ERROR("%s:%s:%d accept err:%d \n", __FILE__, __FUNCTION__, __LINE__, errno);
        if (errno == EMFILE)
        {
            LOG_ERROR("%s:%s:%d socket is limit \n", __FILE__, __FUNCTION__, __LINE__);
        }
    }
}