#ifndef TCP_CONNECTION_H
#define TCP_CONNECTION_H

#include <memory>
#include <string>
#include <atomic>

#include "noncopyable.h"
#include "InetAddress.h"
#include "Callbacks.h"
#include "Buffer.h"
#include "Timestamp.h"



class Channel;
class EventLoop;
class Socket;

class TcpConnection : noncopyable, public std::enable_shared_from_this<TcpConnection>
{
public:
    TcpConnection(EventLoop *loop, 
                const std::string name
                , int sockfd
                , const InetAddress &localAddr
                , const InetAddress &peerAddr);
    ~TcpConnection();

    EventLoop* getLoop() const { return loop_; }
    const std::string& name() const { return name_; }
    const InetAddress& localAddress() const { return localAddr_; }
    const InetAddress& peerAddress() const { return peerAddr_; }

    bool connected() const { return state_ == kConnected; }

    //发送数据
    void send(const void *messsge, int len);
    // 关闭连接
    void shutdown();
    void send(const std::string &buf);

    void setConnectionCallback(const ConnectionCallback &cb) { connectionCallback_ = cb;}
    void setMessageCallback(const MessageCallback &cb) { messageCallback_ = cb; }
    void setWriteCompleteCallback(const WriteCompleteCallback &cb) { writeCompleteCallback_ = cb; }
    void setHighWaterCallback(const HighWaterMarkCallback &cb, size_t highWaterMark) { highWaterMarkCallback_ = cb; highWaterMark_ = highWaterMark; }
    void setCloseCallback(const CloseCallback &cb) { closeCallback_ = cb; }


    void connectionEstablished();
    void connectionDestroyed();
private:
    enum  StateE
    {
        kDisconnected,
        kConnecting,
        kConnected,
        kDisConnecting
    };
    void setState(StateE state) { state_ = state; }
    void handleRead(Timestamp receiveTime);
    void handlerWrite();
    void handlerClose();
    void handlerError();
    void sendInLoop(const std::string &message, size_t len);
    void shutdownInLoop();
private:    
    EventLoop *loop_; // 这里绝对不是mainLoop,而是轮询选择上来的subLoop
    std::string name_;
    std::atomic_int state_; // 连接状态
    bool reading_; //
    
    // 这里和Acceptor类似， Acceptor->mainLoop TcpConnection->subLoop
    std::unique_ptr<Socket> socket_;
    std::unique_ptr<Channel> channel_;
    
    const InetAddress localAddr_; // 服务端
    const InetAddress peerAddr_;  // 客户端

    ConnectionCallback connectionCallback_;
    MessageCallback messageCallback_;
    WriteCompleteCallback writeCompleteCallback_;
    HighWaterMarkCallback highWaterMarkCallback_;
    CloseCallback closeCallback_;
    
    size_t highWaterMark_;

    Buffer inputBuffer_; // read
    Buffer outputBuffer_; // write
};



#endif