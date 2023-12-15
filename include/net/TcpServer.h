#ifndef TCPSERVER_H
#define TCPSERVER_H

#include <functional>
#include <string>
#include <memory>
#include <atomic>
#include <unordered_map>

#include "noncopyable.h"
#include "EventLoop.h"
#include "Acceptor.h"
#include "InetAddress.h"
#include "EventLoopThreadPool.h"
#include "Callbacks.h"
#include "Buffer.h"
#include "TcpConnection.h"


class TcpServer : noncopyable
{
public:
    using ThreadInitCallback = std::function<void(EventLoop *)>;
    using ConnectionMap = std::unordered_map<std::string, TcpConnectionPtr>;

    enum class Option
    {
        kNoReusePort,
        kReusePort
    };

    TcpServer(EventLoop *loop,
              const InetAddress &listenAddr,
              const std::string &nameArg,
              Option option = Option::kNoReusePort);
    ~TcpServer();

    void setThreadInitCallback(const ThreadInitCallback &cb) { threadInitCallback_ = cb; }
    void setConnectionCallback(const ConnectionCallback &cb) { connectionCallback_ = cb; }
    void setMessageCallback(const MessageCallback &cb) { messageCallback_ = cb; }
    void setWriteCompleteCallback(const WriteCompleteCallback &cb) { wrtiteCompleteCallback_ = cb; }
   
   // 设置subLoop的个数
    void setThreadNum(int numThreads);
    // 开启服务器监听
    void start();
private:

    void newConnection(int sockfd, const InetAddress &peerAddr);
    void removeConnection(const TcpConnectionPtr &conn);
    void removeConnectionInLoop(const TcpConnectionPtr &conn);

private:
    EventLoop *loop_; // mainLoop
    const std::string ipPort_;
    const std::string name_;
    std::unique_ptr<Acceptor> acceptor_;              // 运行在mianLoop，任务是监听新连接事件
    // std::shared_ptr<EventLoopThreadPool> threadPool_; // one loop per thread 报错
    EventLoopThreadPool *threadPool_;
    ThreadInitCallback threadInitCallback_;         // loop线程初始化的回调
    ConnectionCallback connectionCallback_;         // 有新连接时的回调
    MessageCallback messageCallback_;               // 有读写消息时的回调
    WriteCompleteCallback wrtiteCompleteCallback_; // 消息发送完成以后的回调

    std::atomic_int start_;
    int nextConnId_; // ?
    ConnectionMap connections_;
};

#endif