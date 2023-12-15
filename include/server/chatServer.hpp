#ifndef CHATSERVER_HPP
#define CHATSERVER_HPP

// #include <muduo/net/TcpServer.h>
// #include <muduo/net/EventLoop.h>
#include "TcpServer.h"
#include <string>

using namespace std;
// using namespace muduo;
// using namespace muduo::net;

class ChatServer
{
public:
    ChatServer(EventLoop *loop, 
            const InetAddress &listenAddr,
            const string &name);
    
    void start();
private:
    void onConnection(const TcpConnectionPtr &conn);
    void onMessage(const TcpConnectionPtr &conn,
            Buffer *buffer,
            Timestamp receiveTime);
private:
    TcpServer _server;
    EventLoop *_loop;
};




#endif