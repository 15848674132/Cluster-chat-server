#ifndef CHATSERVICE_HPP
#define CHATSERVICE_HPP

#include <iostream>
#include <unordered_map>
#include <functional>
#include <memory>   
#include <mutex>
// #include <muduo/net/TcpConnection.h>
#include "TcpConnection.h"

#include <nlohmann/json.hpp>


#include "userModel.hpp"
#include "offlineMessageModel.hpp"
#include "friendModel.hpp"
#include "groupModel.hpp"
#include "redis.hpp"


using namespace std;
// using namespace muduo;
// using namespace muduo::net;

using json = nlohmann::json;
using MsgHandler = std::function<void(const TcpConnectionPtr &conn, json &js, Timestamp receiveTime)>;

// 聊天服务器业务类
class ChatService 
{
public:
    // 获取单例对象 饿汉模式
    static ChatService* getInstance();
    // 处理登录业务
    void login(const TcpConnectionPtr &conn, json &js, Timestamp receiveTime);
    // 处理注册业务
    void reg(const TcpConnectionPtr &conn, json &js, Timestamp receiveTime);
    // 一对一聊天
    void oneChat(const TcpConnectionPtr &conn, json &js, Timestamp receiveTime);
    // 添加好友业务
    void addFriend(const TcpConnectionPtr &conn, json &js, Timestamp receiveTime);
    // 创建群组业务
    void createGroup(const TcpConnectionPtr &conn, json &js, Timestamp receiveTime);
    // 加入群主业务
    void addGroup(const TcpConnectionPtr &conn, json &js, Timestamp receiveTime);
    // 群聊天业务
    void groupChat(const TcpConnectionPtr &conn, json &js, Timestamp receiveTime);
    // 退出登录
    void loginout(const TcpConnectionPtr &conn, json &js, Timestamp receiveTime);
    // 获取消息的处理器
    MsgHandler getHandler(int msgid);
    // 处理客户端异常退出
    void clientCloseException(const TcpConnectionPtr& conn);
    // 服务器异常，业务重置方法
    void reset();
    // 
    void handleRedisSubsribeMessage(int userid, string msg);
private:
    ChatService();
private:
    // 存储消息id和其对应的处理函数
    unordered_map<int, MsgHandler> _msgHandlerMap;
    // 存储在线用户的通信连接 有线程安全问题
    unordered_map<int, TcpConnectionPtr> _userConnMap;
    // 保证_userConnMap的线程安全
    mutex _mutex;
    static ChatService *_chatService;
    // 数据操作对象
    UserModel _userModel;
    OfflineMsgModel _offlineMsgModel;
    FriendModel _friendModel;
    GroupModel _groupModel;
    // redis
    Redis _redis;
};



#endif