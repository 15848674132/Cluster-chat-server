#include "TcpConnection.h"
#include "Logger.h"
#include "Socket.h"
#include "Channel.h"
#include "EventLoop.h"

// #ifdef MUDEBUG
#include <string.h>
// #endif

#include <functional>
#include <string>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/tcp.h>
#include <unistd.h>

static EventLoop *CheackLoopNotNull(EventLoop *loop)
{
    if (loop == nullptr)
    {
        LOG_FATAL("%s:%s:%d mainLoop is null!\n", __FILE__, __FUNCTION__, __LINE__);
    }

    return loop;
}

TcpConnection::TcpConnection(EventLoop *loop,
                             const std::string name, int sockfd, const InetAddress &localAddr, const InetAddress &peerAddr)
    : loop_(CheackLoopNotNull(loop))
    , name_(name), state_(kConnecting)
    , reading_(true)
    , socket_(new Socket(sockfd))
    , channel_(new Channel(loop, sockfd))
    , localAddr_(localAddr)
    , peerAddr_(peerAddr)
    , highWaterMark_(64 * 1024 * 1024)
{
    // 设置channel的回调
    channel_->setReadCallback(std::bind(&TcpConnection::handleRead, this, std::placeholders::_1));
    channel_->setWriteCallback(std::bind(&TcpConnection::handlerWrite, this));
    channel_->setCloseCallback(std::bind(&TcpConnection::handlerClose, this));
    channel_->setErrorCallback(std::bind(&TcpConnection::handlerError, this));

    LOG_INFO("%s:%s:%d TcpConnection::ctor[%s] at %p clientfd=%d",
            __FILE__, __FUNCTION__, __LINE__,
            name_.c_str(), this, sockfd);
}

TcpConnection::~TcpConnection()
{
    LOG_INFO("%s:%s:%d TcpConnection::dtor[%s] at clientfd=%d state=%d",
            __FILE__, __FUNCTION__, __LINE__,
            name_.c_str(),
            channel_->fd(), static_cast<int>(state_));
}

void TcpConnection::handleRead(Timestamp receiveTime)
{
    int saveErrno = 0;
    ssize_t n = inputBuffer_.readFd(channel_->fd(), &saveErrno);

    if (n > 0)
    {
        // 用户设置对读取上来的数据进行怎么样的处理
        messageCallback_(shared_from_this(), &inputBuffer_, receiveTime);
    }
    else if (n == 0)
    {
        handlerClose();
    }
    else
    {
        errno = saveErrno;
        LOG_ERROR("%s:%s:%d TcpConnection::handleRead error\n",
        __FILE__, __FUNCTION__, __LINE__);
        handlerError();
    }
}

void TcpConnection::handlerWrite()
{
    if (channel_->isWriting())
    {
        int saveErrno = 0;
        ssize_t n = outputBuffer_.writeFd(channel_->fd(), &saveErrno);
        if (n > 0)
        {
            outputBuffer_.retrive(n);
            if (outputBuffer_.readableBytes() == 0)
            {
                channel_->disalbeWriting();
                if (writeCompleteCallback_)
                {
                    //
                    loop_->queueInLoop(
                        std::bind(writeCompleteCallback_, shared_from_this()));
                }

                if (state_ == kDisConnecting)
                {
                    shutdownInLoop();
                }
            }
            else
            {
                LOG_ERROR("%s:%s:%d TcpConnection::handlerWrite error\n",
                __FILE__, __FUNCTION__, __LINE__);
            }
        }
    }
    else
    {
        LOG_ERROR("%s:%s:%d TcpConnection fd=%d is down, no more writing \n", 
        __FILE__, __FUNCTION__, __LINE__, channel_->fd());
    }
}

void TcpConnection::handlerClose()
{
    LOG_DEBUG("%s:%s:%d handlerClose fd=%d state=%d \n", 
        __FILE__, __FUNCTION__, __LINE__,
        channel_->fd(), static_cast<int>(state_));
    setState(kDisconnected);
    // 从poller中移除注册channel
    channel_->disableAll();

    TcpConnectionPtr connPtr(shared_from_this());
    // 用户设置的回调
    connectionCallback_(connPtr);
    // TcpServer设置的回调，执行在TcpServer中删除该连接，执行的时TcpConnection::connectionDestroyed
    // 从poller中移除为注册的channel
    closeCallback_(connPtr);
}

void TcpConnection::handlerError()
{
    int optval; 
    socklen_t optlen = sizeof optval;
    int saveErrno = 0;
    if (::getsockopt(channel_->fd(), SOL_SOCKET, SO_ERROR, &optval, &optlen))
    {
        saveErrno = errno;
    }
    else
    {
        saveErrno = optval;
    }
    LOG_ERROR("TcpConnection::handlerError name:%s - SO_ERROR:%d \n", name_.c_str(), saveErrno);
}

// void TcpConnection::sendInLoop(const void *message, size_t len)
void TcpConnection::sendInLoop(const std::string &message, size_t len)
{
    ssize_t nwrote = 0;
    size_t remaining = len;
    bool faultError = false;

    if (state_ == kDisconnected)
    {
        LOG_ERROR("disconnected, give up writing!\n");
        return;
    }

    // 表示channel第一次写数据， 而且缓冲区没有数据
    if (!channel_->isWriting() && outputBuffer_.readableBytes() == 0)
    {
        char *begin = (char*)message.c_str();
        // char *begin = (char*)message;
        LOG_INFO("TcpConnection::sendInLoop :%ld %s", len, begin);


        // 这里发生数据丢失
        nwrote = ::write(channel_->fd(), message.c_str(), len);
        if (nwrote >= 0)
        {
            remaining = len - nwrote;
            if (remaining == 0 && writeCompleteCallback_)
            {
                // 既然在这里一次性发送完了，就不用在给channel设置epollout事件了
                loop_->queueInLoop(
                    std::bind(writeCompleteCallback_, shared_from_this()));
            }
        }
        else
        {
            nwrote = 0;
            if (errno != EWOULDBLOCK) // EAGAIN
            {
                LOG_ERROR("TcpConnection::sendInLoop\n");
                if (errno == EPIPE || errno == ECONNRESET)
                {
                    faultError = true;
                }
            }
        }
    }
    // 说明当前这一次write，并没有把数据全部发送出去，剩余的数据需要保存在缓冲区当中
    // 然后给channel注册epollout事件，poller监听tcp发送缓冲区有空间，会通知channel
    // 调用handevent处理write事件
    if (!faultError && remaining > 0)
    {
        // 目前发送缓冲区剩余的待发送数据的长度
        size_t oldLen = outputBuffer_.readableBytes();
        if (oldLen + remaining >= highWaterMark_ && oldLen < highWaterMark_ && highWaterMarkCallback_)
        {
            loop_->queueInLoop(
                std::bind(highWaterMarkCallback_, shared_from_this(), oldLen + remaining));
        }

        outputBuffer_.append(message.c_str() + nwrote, remaining);
        // outputBuffer_.append((char*)message + nwrote, remaining);
        if (!channel_->isWriting())
        {
            channel_->enalbeWriting();
        }
    }
}

void TcpConnection::send(const std::string &buf)
{
    if (state_ == kConnected)
    {
        char *begin = (char*)buf.c_str();
        LOG_INFO("%s:%s:%d send is %s", __FILE__, __FUNCTION__, __LINE__, begin);
        if (loop_->isInLoopThread())
        {
            sendInLoop(buf, buf.size());
        }
        else
        {
            loop_->runInLoop(std::bind(
                &TcpConnection::sendInLoop, this, buf, buf.size()));
        }
    }
}

void TcpConnection::connectionEstablished()
{
    setState(kConnected);
    channel_->tie(shared_from_this());
    channel_->enableReading();
    // 新连接建立，执行回调
    connectionCallback_(shared_from_this()); // 执行用户设置的回调函数
}

void TcpConnection::connectionDestroyed()
{
    if (state_ == kConnected)
    {
        setState(kDisconnected);
        channel_->disableAll(); // 从epoll中删除

        connectionCallback_(shared_from_this());
    }

    channel_->remove(); // 从poller中删除
}

void TcpConnection::shutdown()
{
    if (state_ == kConnected)
    {
        setState(kDisConnecting);
        loop_->runInLoop(
            std::bind(&TcpConnection::shutdownInLoop, this));
    }
}

void TcpConnection::shutdownInLoop()
{
    if (!channel_->isWriting())
    {
        socket_->shutDownWrite();
    }
}
