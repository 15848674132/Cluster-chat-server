#include "TcpServer.h"
#include "Logger.h"

#include <string.h>

static EventLoop *CheackLoopNotNull(EventLoop *loop)
{
    if (loop == nullptr)
    {
        LOG_FATAL("%s:%s:%d mainLoop is null!\n", __FILE__, __FUNCTION__, __LINE__);
    }

    return loop;
}

TcpServer::TcpServer(EventLoop *loop,
                     const InetAddress &listenAddr,
                     const std::string &nameArg,
                     Option option)
    : loop_(CheackLoopNotNull(loop)), ipPort_(listenAddr.toIpPort()), name_(nameArg), acceptor_(new Acceptor(loop, listenAddr, option == Option::kReusePort)) // 重要
      ,
      threadPool_(new EventLoopThreadPool(loop, name_)), connectionCallback_(), messageCallback_(), start_(0), nextConnId_(1)
{
    // threadPool_ = std::make_shared<EventLoopThreadPool>(loop, name_);
    acceptor_->setNewConnectCallback(std::bind(&TcpServer::newConnection, this,
                                               std::placeholders::_1, std::placeholders::_2));
}

TcpServer::~TcpServer()
{
    for (auto &item : connections_)
    {
        TcpConnectionPtr conn(item.second);
        item.second.reset(); // reset是智能指针的方法，重置智能指针

        // 销毁连接
        conn->getLoop()->runInLoop(
            std::bind(&TcpConnection::connectionDestroyed, conn));
    }
}

// 设置subLoop的个数
void TcpServer::setThreadNum(int numThreads)
{
    threadPool_->setThreadNum(numThreads);
}
// 开启服务器监听
void TcpServer::start()
{
    // LOG_INFO("%s:%s:%d begin start", __FILE__, __FUNCTION__, __LINE__);

    if (start_++ == 0) // 防止一个TcpServer对象启动多次
    {
        // LOG_INFO("%s:%s:%d start", __FILE__, __FUNCTION__, __LINE__);

        // 启动线程池 -> 还有细节
        threadPool_->start(threadInitCallback_);
        // 将accpetChannel注册在mianLoop中的poller上面
        loop_->runInLoop(std::bind(&Acceptor::listen, acceptor_.get()));
    }
}

// 有一个新的客户端连接，acceptor会执行这个回调
void TcpServer::newConnection(int sockfd, const InetAddress &peerAddr)
{
    // 轮询算法，选择一个subLoop，来管理channel
    EventLoop *ioLoop = threadPool_->getNextLoop();
    char buf[64];
    snprintf(buf, sizeof buf, "-%s#%d", ipPort_.c_str(), nextConnId_);
    ++nextConnId_; // 不需要原子变量，因为只在mianLoop中执行newConnection
    std::string connName = name_ + buf;

    LOG_INFO("%s:%s:%d TcpServer::newConnection [%s] - new connection [%s] from %s",
             __FILE__, __FUNCTION__, __LINE__,
             name_.c_str(), connName.c_str(), peerAddr.toIpPort().c_str());

    // 通过sockfd获取其绑定的本机的ip地址和端口信息
    sockaddr_in local;
    memset(&local, 0, sizeof local);
    socklen_t addrLen = sizeof local;
    if (::getsockname(sockfd, (sockaddr *)&local, &addrLen) < 0)
    {
        LOG_ERROR("%s:%s:%d sockets::getLocalAddr error",
                  __FILE__, __FUNCTION__, __LINE__);
    }

    InetAddress localAddr(local);

    // 根据连接成功的sockfd，创建TcpConnection连接对象 并且设置了channel的回调
    TcpConnectionPtr conn(new TcpConnection(
        ioLoop,
        connName,
        sockfd,
        localAddr,
        peerAddr));

    connections_[connName] = conn;
    // 下面都是用户设置给TcpServer TcpServer->TcpConnection->Channel->poller->notify不是条件变量 channel调用回调
    conn->setConnectionCallback(connectionCallback_);
    conn->setMessageCallback(messageCallback_);
    conn->setWriteCompleteCallback(wrtiteCompleteCallback_);
    // 设置如何关闭的回调
    conn->setCloseCallback(
        std::bind(&TcpServer::removeConnection, this, std::placeholders::_1));

    ioLoop->runInLoop(std::bind(&TcpConnection::connectionEstablished, conn));
}

void TcpServer::removeConnection(const TcpConnectionPtr &conn)
{
    loop_->runInLoop(
        std::bind(&TcpServer::removeConnectionInLoop, this, conn));
}

void TcpServer::removeConnectionInLoop(const TcpConnectionPtr &conn)
{
    LOG_INFO("%s:%s:%d TcpServer::removeConnectionInLoop [%s] - connection %s",
             __FILE__, __FUNCTION__, __LINE__,
             name_.c_str(), conn->name().c_str());

    connections_.erase(conn->name());
    EventLoop *ioLoop = conn->getLoop();
    ioLoop->queueInLoop(
        std::bind(&TcpConnection::connectionDestroyed, conn));
}