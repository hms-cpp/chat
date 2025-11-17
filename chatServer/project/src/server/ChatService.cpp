#include "ChatService.hpp"
#include "public.hpp"
#include "User.hpp"

#include <string>
#include <queue>
#include <muduo/base/Logging.h>

#include <vector>
#include <unordered_map>
#include <iostream>

ChatService::ChatService()
{
    _msgHanderMap.insert({SHUT_DONW, std::bind(&ChatService::shutdown, this,
                                               std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)});

    _msgHanderMap.insert({LOGIN_MSG, std::bind(&ChatService::login, this,
                                               std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)});

    _msgHanderMap.insert({REG_MSG, std::bind(&ChatService::regist, this,
                                             std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)});

    _msgHanderMap.insert({ONE_CHAT_MSG, std::bind(&ChatService::oneChat, this,
                                                  std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)});
    _msgHanderMap.insert({ADD_FRIEND_MSG, std::bind(&ChatService::addFriend, this,
                                                    std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)});
    _msgHanderMap.insert({CREATE_GROUP_MSG, std::bind(&ChatService::createGroup, this,
                                                      std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)});
    _msgHanderMap.insert({ADD_GROUP_MSG, std::bind(&ChatService::addGroup, this,
                                                   std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)});
    _msgHanderMap.insert({GROUP_CHAT_MSG, std::bind(&ChatService::GroupChat, this,
                                                    std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)});

    _msgHanderMap.insert({LOGIN_OUT_MSG, std::bind(&ChatService::loginout, this,
                                                   std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)});

    if (_redis.connect())
    {
        _redis.init_notify_handler(std::bind(&ChatService::handeRedisSubscribeMessage, this,
                                             std::placeholders::_1, std::placeholders::_2));
    }

    _msgHanderMap.insert({MARCH_MSG, std::bind(&ChatService::march, this,
                                                   std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)});
}

ChatService *ChatService::instance()
{
    static ChatService service;
    return &service;
}

void ChatService::reset()
{
    // 重置用户登录状态
    _usermodel.reset_user_state();
}

void ChatService::defaultHandler(const muduo::net::TcpConnectionPtr &conn, json &js, muduo::Timestamp time)
{
    return;
}

void ChatService::shutdown(const muduo::net::TcpConnectionPtr &conn, json &js, muduo::Timestamp time)
{
    std::cout << "用户连接关闭" << std::endl;
}

void ChatService::loginout(const muduo::net::TcpConnectionPtr &conn, json &js, muduo::Timestamp time)
{
    int id = js["id"].get<int>();
    if (id == 0)
    {
        return;
    }
    else
    {
        _redis.unsubscribe(id);
        _userConnectionMap.erase(id);
        _usermodel.update_user_state_for_client_close(id);
    }
}

void ChatService::login(const muduo::net::TcpConnectionPtr &conn, json &js, muduo::Timestamp time)
{
    int id = js["id"].get<int>();
    std::string pwd = js["password"];

    User user = _usermodel.query(id);
    if (user.getPassword() == pwd)
    {
        // 重复登录
        if (user.getState() == "online")
        {
            json response;
            response["msg_id"] = LOGIN_MSG_ACK;
            response["errno"] = 2;
            response["error_msg"] = "该用户已登录";
            conn->send(response.dump());

            return;
        }
        // 登录成功

        // 开启订阅
        _redis.subscribe(id);

        json response;
        response["msg_id"] = LOGIN_MSG_ACK;
        response["errno"] = 0;
        response["id"] = user.getId();
        response["name"] = user.getName();

        // 更新用户状态
        user.setState("online");
        _usermodel.update_user_state(user);

        // 记录用户连接信息
        std::unique_lock<std::mutex> lock(_connMutex);
        _userConnectionMap.insert({id, conn});

        // 后续可以设置成手动查看离线消息和手动删除离线消息

        // 查询离线消息
        std::vector<std::string> vec = _offlineMsgModel.query(id);
        if (!vec.empty())
        {
            // 发送离线消息
            response["offline_msg"] = vec;

            // 清除离线消息
            _offlineMsgModel.remove(id);
        }

        // 查询用户好友信息
        std::vector<User> user_vec = _friendModel.query(id);
        if (!user_vec.empty())
        {
            debug_cout("检查用户:", id, "好友信息", "信息不为空");
            // 发送好友消息
            std::vector<std::string> vec2;
            for (User &user : user_vec)
            {
                json js;
                js["id"] = user.getId();
                js["name"] = user.getName();
                js["state"] = user.getState();
                vec2.push_back(js.dump());
            }
            response["friend"] = vec2;
        }

        // 查询用户群组信息
        std::vector<Group> groupUserVec = _groupModel.queryGroups(id);
        if (!groupUserVec.empty())
        {
            debug_cout("检查用户:", id, "群组信息", "信息不为空");
            std::vector<std::string> groupV;
            for (Group &group : groupUserVec)
            {
                json groupJs;
                groupJs["id"] = group.getId();
                groupJs["group_name"] = group.getName();
                groupJs["group_desc"] = group.getDesc();
                std::vector<std::string> userV;
                for (GroupUser &user : group.getUsers())
                {
                    json js;
                    js["id"] = user.getId();
                    js["name"] = user.getName();
                    js["state"] = user.getState();
                    js["role"] = user.getRole();
                    userV.push_back(js.dump());
                }
                groupJs["users"] = userV;
                groupV.push_back(groupJs.dump());
            }
            response["groups"] = groupV;
        }

        conn->send(response.dump());
        debug(response.dump());
        return;
    }
    else
    {
        // 登录失败
        json response;
        response["msg_id"] = LOGIN_MSG_ACK;
        response["errno"] = 1;
        response["error_msg"] = "用户名或密码错误";
        conn->send(response.dump());
    }

    LOG_INFO << "do login service!";
}

void ChatService::regist(const muduo::net::TcpConnectionPtr &conn, json &js, muduo::Timestamp time)
{
    std::string name = js["name"];
    std::string pwd = js["password"];

    User user;
    user.setName(name);
    user.setPassword(pwd);
    bool state = _usermodel.insertUser(user);
    if (state)
    {
        json response;
        response["msg_id"] = REG_MSG_ACK;
        response["errno"] = 0;
        response["name"]=name;
        response["password"]=pwd;
        response["id"] = user.getId();
        conn->send(response.dump());
    }
    else
    {
        json response;
        response["msg_id"] = REG_MSG_ACK;
        response["errno"] = 1;
        response["name"]=name;
        conn->send(response.dump());
    }

    LOG_INFO << "do reg service!";
}

void ChatService::oneChat(const muduo::net::TcpConnectionPtr &conn, json &js, muduo::Timestamp time)
{
    int toid = js["to"].get<int>();

    std::string str = js.dump();

    debug(str);

    // 判断是否在本机并处理
    {
        std::lock_guard<std::mutex> lock(_connMutex);
        auto it = _userConnectionMap.find(toid);
        if (it != _userConnectionMap.end())
        {
            it->second->send(str);
            return;
        }
    }

    // 如果不在本机，查询对方在线情况
    User user = _usermodel.query(toid);
    // 如果在线证明在其他服务器上登陆了
    if (user.getState() == "online")
    {
        // 发布消息到redis
        _redis.publish(toid, str);
        return;
    }

    // 对方不在线,记录离线消息
    _offlineMsgModel.insert(toid, str);
    return;
}

void ChatService::addFriend(const muduo::net::TcpConnectionPtr &conn, json &js, muduo::Timestamp time)
{
    int user_id = js["id"].get<int>();
    int friend_id = js["friend_id"].get<int>();

    _friendModel.insert(user_id, friend_id);
}

void ChatService::createGroup(const muduo::net::TcpConnectionPtr &conn, json &js, muduo::Timestamp time)
{
    int user_id = js["id"].get<int>();
    std::string name = js["group_name"];
    std::string desc = js["group_desc"];

    Group group(-1, name, desc);
    if (_groupModel.createGroup(group))
    {
        _groupModel.addToGroup(user_id, group.getId(), "creator");
    }
}

void ChatService::addGroup(const muduo::net::TcpConnectionPtr &conn, json &js, muduo::Timestamp time)
{
    int user_id = js["id"].get<int>();
    int group_id = js["group_id"].get<int>();
    _groupModel.addToGroup(user_id, group_id, "normal");
}

void ChatService::GroupChat(const muduo::net::TcpConnectionPtr &conn, json &js, muduo::Timestamp time)
{
    int user_id = js["id"].get<int>();
    int group_id = js["group_id"].get<int>();
    std::string str=js.dump();
    std::vector<int> user_id_inGroup = _groupModel.queryGroupUsers(user_id, group_id);
    std::lock_guard<std::mutex> lock(_connMutex);
    for (int id : user_id_inGroup)
    {
        auto it = _userConnectionMap.find(id);
        if (it != _userConnectionMap.end())
        {
            it->second->send(str);
        }
        else
        {
            User user=_usermodel.query(id);
            if(user.getState()=="online")
            {
                _redis.publish(id,str);
            }
            else
            {
                _offlineMsgModel.insert(id,str);
            }
        }
    }
}

MsgHandler ChatService::getHandler(int msg_id)
{
    auto it = _msgHanderMap.find(msg_id);
    if (it == _msgHanderMap.end())
    {
        LOG_ERROR << "message id : " << msg_id << "can not find handler!";

        // 返回默认处理器
        return std::bind(&ChatService::defaultHandler, this,
                         std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
    }
    else
    {
        return _msgHanderMap[msg_id];
    }
}

void ChatService::clientCloseException(const muduo::net::TcpConnectionPtr &conn)
{
    LOG_DEBUG << "Cleaning up connection: " << conn->name();

    std::lock_guard<std::mutex> lock(_connMutex);

    for (auto it = _userConnectionMap.begin(); it != _userConnectionMap.end();)
    {
        if (it->second == conn)
        {
            int userid = it->first;
            LOG_INFO << "User " << userid << " disconnected, updating state";

            // 取消订阅
            _redis.unsubscribe(userid);

            // 更新用户状态
            _usermodel.update_user_state_for_client_close(userid);

            // 安全删除
            it = _userConnectionMap.erase(it);

            LOG_INFO << "User " << userid << " cleanup completed";
        }
        else
        {
            ++it;
        }
    }
}

void ChatService::handeRedisSubscribeMessage(int user_id, string msg)
{
    std::lock_guard<std::mutex> lock(_connMutex);
    auto it=_userConnectionMap.find(user_id);
    if(it!=_userConnectionMap.end())
    {
        it->second->send(msg);
        return ;
    }
    
    _offlineMsgModel.insert(user_id,msg);
}

void ChatService::march(const muduo::net::TcpConnectionPtr &conn, json &js, muduo::Timestamp time)
{
    User user=_usermodel.find_for_march(js["id"].get<int>());
    json res;
    res["msg_id"]=MARCH_MSG;
    res["marched_id"]=user.getId();
    res["to"]=js["id"].get<int>();
    oneChat(conn,res,time);

    res["to"]=user.getId();
    res["marched_id"]=js["id"].get<int>();
    oneChat(conn,res,time);
}
