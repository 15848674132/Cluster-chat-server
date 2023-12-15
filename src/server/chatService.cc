#include "chatService.hpp"
#include "public.hpp"
// #include <muduo/base/Logging.h>
#include "Logger.h"
#include <string>
// #include <map>
using namespace std;
using namespace std::placeholders;
// using namespace muduo;

// 饿汉模式
ChatService *ChatService::getInstance()
{
    return _chatService;
}

ChatService *ChatService::_chatService = new ChatService();

ChatService::ChatService()
{
    _msgHandlerMap.insert({LOGIN_MSG, bind(&ChatService::login, this, _1, _2, _3)});
    _msgHandlerMap.insert({REG_MSG, bind(&ChatService::reg, this, _1, _2, _3)});
    _msgHandlerMap.insert({ONE_CHAT_MSG, bind(&ChatService::oneChat, this, _1, _2, _3)});
    _msgHandlerMap.insert({ADD_FRIEND_MSG, bind(&ChatService::addFriend, this, _1, _2, _3)});
    _msgHandlerMap.insert({CREATE_GROUP_MSG, bind(&ChatService::createGroup, this, _1, _2, _3)});
    _msgHandlerMap.insert({ADD_GROUP_MSG, bind(&ChatService::addGroup, this, _1, _2, _3)});
    _msgHandlerMap.insert({GROUP_CHAT_MSG, bind(&ChatService::groupChat, this, _1, _2, _3)});
    _msgHandlerMap.insert({LOGINOUT_MSG, bind(&ChatService::loginout, this, _1, _2, _3)});

    // 连接reids服务器
    if (_redis.connect())
    {
        // 设置上报消息回调
        _redis.init_notify_handler(std::bind(&ChatService::handleRedisSubsribeMessage, this, _1, _2));
    }
}

MsgHandler ChatService::getHandler(int msgid)
{
    auto it = _msgHandlerMap.find(msgid);
    if (it == _msgHandlerMap.end())
    {
        // 返回一个默认的处理器，空操作
        return [=](const TcpConnectionPtr &, json &, Timestamp)
        {
            // LOG_ERROR << "msgid: " << msgid << " can not find handler!";
        };
    }
    else
    {
        return _msgHandlerMap[msgid];
    }
}

void ChatService::login(const TcpConnectionPtr &conn, json &js, Timestamp receiveTime)
{
    // LOG_INFO << "do login service!";
    LOG_INFO("do login service!");

    int id = js["id"].get<int>();
    string password = js["password"];
    User user = _userModel.query(id);

    json response;
    response["msgid"] = LOGIN_MSG_ACK;
    if (user.getId() != -1 && user.getPassword() == password)
    {
        if (user.getState() == "online")
        {
            // 不允许重复登录
            response["errno"] = 2;
            response["errmsg"] = "this account is using, input another!";
        }
        else
        {
            user.setState("online");
            bool state = _userModel.update(user);
            // 登录成功
            if (state)
            {
                // 保存登录成功连接
                {
                    lock_guard<mutex> lock(_mutex);
                    _userConnMap.insert({id, conn});
                }

                // id用户登录成功后，向redis订阅channel(id)
                _redis.subscribe(id);

                response["errno"] = 0;
                response["id"] = user.getId();
                response["name"] = user.getName();

                // 查看是否有离线消息
                vector<string> vec = _offlineMsgModel.query(id);
                if (!vec.empty())
                {
                    response["offlinemsg"] = vec;
                    // 读取后删除离线消息
                    _offlineMsgModel.remove(id);
                }

                // 查看该用户的好友信息并返回
                vector<User> friendVec = _friendModel.query(id);
                if (!friendVec.empty())
                {
                    vector<string> vec2;
                    for (auto &user : friendVec)
                    {
                        json js;
                        js["id"] = user.getId();
                        js["name"] = user.getName();
                        js["state"] = user.getState();
                        vec2.push_back(js.dump());
                    }
                    response["friends"] = vec2;
                }

                // 查看用户的群组消息
                vector<Group> groupsuerVec = _groupModel.queryGroups(id);
                if (!groupsuerVec.empty())
                {
                    vector<string> groupV;
                    for (auto &group : groupsuerVec)
                    {
                        json grpjson;
                        grpjson["id"] = group.getId();
                        grpjson["groupname"] = group.getName();
                        grpjson["groupdesc"] = group.getDesc();
                        vector<string> userV;
                        for (auto &user : group.getUsers())
                        {
                            json js;
                            js["id"] = user.getId();
                            js["name"] = user.getName();
                            js["state"] = user.getState();
                            js["role"] = user.getRole();
                            userV.push_back(js.dump());
                        }
                        grpjson["users"] = userV;
                        groupV.push_back(grpjson.dump());
                    }
                    response["groups"] = groupV;
                }
            }
            else
            {
                user.setState("offline");
                response["errno"] = 3;
                response["errmsg"] = "login failure!";
            }
        }
    }
    else
    {
        response["errno"] = 1;
        response["errmsg"] = "The user name or password is incorrect!";
    }

    conn->send(response.dump());
}

void ChatService::loginout(const TcpConnectionPtr &conn, json &js, Timestamp receiveTime)
{
    // LOG_INFO << "do loginout service!";
    LOG_INFO("do loginout service!");

    int userid = js["id"].get<int>();
    User user;
    {
        lock_guard<mutex> lock(_mutex);
        auto it = _userConnMap.find(userid);
        if (it != _userConnMap.end())
        {
            user.setId(it->first);
            _userConnMap.erase(it);
        }
    }

    // 用户下线取消在redis中的订阅
    _redis.unsubscribe(userid);

    // 更新用户的状态信息
    if (user.getId() != -1)
    {
        user.setState("offline");
        _userModel.update(user);
    }
}

void ChatService::reg(const TcpConnectionPtr &conn, json &js, Timestamp receiveTime)
{
    // LOG_INFO << "do reg service!";
    LOG_INFO("do reg service!");

    string name = js["name"];
    string password = js["password"];

    User user;
    user.setName(name);
    user.setPassword(password);
    bool state = _userModel.insert(user);

    json response;
    response["msgid"] = REG_MSG_ACK;
    if (state)
    {
        // 注册成功
        response["errno"] = 0;
        response["id"] = user.getId();
        response["name"] = user.getName();
    }
    else
    {
        // 注册失败
        response["errno"] = 1;
    }
    conn->send(response.dump());
}

void ChatService::oneChat(const TcpConnectionPtr &conn, json &js, Timestamp receiveTime)
{
    /**
     * msgid:ONE_CHAT_MSG
     * id:1
     * from："zhang san"
     * to:3
     * msg:xxx
     */
    // LOG_INFO << "do oneChat service!";
    LOG_INFO("do oneChat service!");
    
    int toid = js["to"].get<int>();
    {
        lock_guard<mutex> lock(_mutex);
        auto it = _userConnMap.find(toid);
        if (it != _userConnMap.end())
        {
            // toid在线。消息转发
            it->second->send(js.dump());
        }
        else
        {
            // 用户在其他服务器上面在线
            User user = _userModel.query(toid);
            if (user.getState() == "online")
            {
                cout << "_redis.publish " << js.dump() << endl;

                _redis.publish(toid, js.dump());
            }
            else
            {
                // toid不在线，存储离线消息
                _offlineMsgModel.insert(toid, js.dump());
            }
        }
    }
}

void ChatService::addFriend(const TcpConnectionPtr &conn, json &js, Timestamp receiveTime)
{
    // LOG_INFO << "do addFrined service!";
    LOG_INFO("do addFrined service!");

    int userid = js["id"].get<int>();
    int friendid = js["friendid"].get<int>();
    _friendModel.insert(userid, friendid);
}

void ChatService::createGroup(const TcpConnectionPtr &conn, json &js, Timestamp receiveTime)
{
    // LOG_INFO << "do createGroup service!";
    LOG_INFO("do createGroup service!");
    int userid = js["id"].get<int>();
    Group group;
    group.setName(js["groupname"].get<string>());
    group.setDesc(js["groupdesc"].get<string>());
    if (_groupModel.createGroup(group))
    {
        _groupModel.addGroup(userid, group.getId(), "creator");
    }
}

void ChatService::addGroup(const TcpConnectionPtr &conn, json &js, Timestamp receiveTime)
{
    // LOG_INFO << "do addGroup service!";
    LOG_INFO("do addGroup service!");
    int userid = js["userid"].get<int>();
    int groupid = js["groupid"].get<int>();
    _groupModel.addGroup(userid, groupid);
}

void ChatService::groupChat(const TcpConnectionPtr &conn, json &js, Timestamp receiveTime)
{
    /**
     * msgid:GROUP_CHAT_MSG
     * id:1
     * from："zhang san"
     * groupid:2
     * msg:xxx
     */
    int userid = js["id"].get<int>();
    int groupid = js["groupid"].get<int>();
    vector<int> useridvec = _groupModel.queryGroupUsers(userid, groupid);
    lock_guard<mutex> lock(_mutex); // 感觉锁应该在往上面放一行
    for (auto id : useridvec)
    {
        auto it = _userConnMap.find(id);
        if (it != _userConnMap.end())
        {
            it->second->send(js.dump());
        }
        else
        {
            // 用户在其他服务器上面在线
            User user = _userModel.query(id);
            if (user.getState() == "online")
            {
                cout << "_redis.publish " << js.dump() << endl;
                _redis.publish(id, js.dump());
            }
            else
            {
                // toid不在线，存储离线消息
                _offlineMsgModel.insert(id, js.dump());
            }
        }
    }
}

void ChatService::clientCloseException(const TcpConnectionPtr &conn)
{
    User user;
    {
        lock_guard<mutex> lock(_mutex);
        for (auto it = _userConnMap.begin(); it != _userConnMap.end(); ++it)
        {
            if (it->second == conn)
            {
                // 从map表删除用户的连接信息
                user.setId(it->first);
                _userConnMap.erase(it);
                break;
            }
        }
    }

    // 用户下线取消在redis中的订阅
    _redis.unsubscribe(user.getId());

    // 更新用户的状态信息
    if (user.getId() != -1)
    {
        user.setState("offline");
        _userModel.update(user);
    }
}

void ChatService::reset()
{
    // 把online状态的用户设置为offline
    _userModel.resetState();
}

void ChatService::handleRedisSubsribeMessage(int userid, string msg)
{
    lock_guard<mutex> lock(_mutex);
    auto it = _userConnMap.find(userid);
    if(it != _userConnMap.end())
    {
        cout << "handleRedisSubsribeMessage : " << msg << endl;
        it->second->send(msg);
    }
    else
    {
        _offlineMsgModel.insert(userid, msg);
    } 
}