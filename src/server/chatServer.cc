#include "chatServer.hpp"
#include <functional>
#include <string>
#include <nlohmann/json.hpp>
#include "chatService.hpp"

using namespace std;
using namespace placeholders;
using json = nlohmann::json;

ChatServer::ChatServer(EventLoop *loop,
                       const InetAddress &listenAddr,
                       const string &name)
    : _server(loop, listenAddr, name), _loop(loop)
{
    // 设置处理连接建立和断开的回调函数
    _server.setConnectionCallback(bind(&ChatServer::onConnection, this, _1));

    // 设置对已读取的数据的处理回调函数
    _server.setMessageCallback(bind(&ChatServer::onMessage, this, _1, _2, _3));

    // 设置subLoopThread的个数
    _server.setThreadNum(4);
}

void ChatServer::start()
{
    // 启动subLoopThread
    _server.start();
}

// 处理连接建立和断开的函数
void ChatServer::onConnection(const TcpConnectionPtr &conn)
{
    if (!conn->connected())
    {
        ChatService::getInstance()->clientCloseException(conn);
        conn->shutdown();
    }
}

// 对已读取的数据的处理函数
void ChatServer::onMessage(const TcpConnectionPtr &conn,
                           Buffer *buffer,
                           Timestamp receiveTime)
{
    // 从读缓存区拿取数据
    string buf = buffer->retrieveAllAsString();
    // 数据反序列化
    json js = json::parse(buf);
    // 完全解耦网络模块和业务模块的代码 oop:回调 or 面向接口
    auto msgHandler = ChatService::getInstance()->getHandler(js["msgid"].get<int>());
    // 回调消息绑定好的事件处理器，来执行相应的业务处理
    msgHandler(conn, js, receiveTime);
}
