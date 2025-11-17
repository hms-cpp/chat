#pragma once

#include <muduo/net/TcpConnection.h>
#include <unordered_map>
#include <functional>
#include <memory>
#include <mutex>

#include "UserModel.hpp"
#include "OfflineMessageModel.hpp"
#include "FriendModel.hpp"
#include "json.hpp"
#include "GroupModel.hpp"
#include "redis.hpp"

using json = nlohmann::json;

using MsgHandler = std::function<void(const muduo::net::TcpConnectionPtr &conn, json &js, muduo::Timestamp time)>;

class ChatService
{
public:
    static ChatService *instance();

    // 默认处理器
    void defaultHandler(const muduo::net::TcpConnectionPtr &conn, json &js, muduo::Timestamp time);

    void shutdown(const muduo::net::TcpConnectionPtr &conn, json &js, muduo::Timestamp time);

    void loginout(const muduo::net::TcpConnectionPtr &conn, json &js, muduo::Timestamp time);

    // 登录
    void login(const muduo::net::TcpConnectionPtr &conn, json &js, muduo::Timestamp time);

    // 注册
    void regist(const muduo::net::TcpConnectionPtr &conn, json &js, muduo::Timestamp time);

    // 一对一聊天
    void oneChat(const muduo::net::TcpConnectionPtr &conn, json &js, muduo::Timestamp time);

    //
    void addFriend(const muduo::net::TcpConnectionPtr &conn, json &js, muduo::Timestamp time);

    // 创建群组
    void createGroup(const muduo::net::TcpConnectionPtr &conn, json &js, muduo::Timestamp time);

    // 加入群
    void addGroup(const muduo::net::TcpConnectionPtr &conn, json &js, muduo::Timestamp time);

    // 群聊
    void GroupChat(const muduo::net::TcpConnectionPtr &conn, json &js, muduo::Timestamp time);

    // 获取消息对应的处理器
    MsgHandler getHandler(int msg_id);

    void clientCloseException(const muduo::net::TcpConnectionPtr &conn);

    void handeRedisSubscribeMessage(int,string);

    //
    void reset();

    void march(const muduo::net::TcpConnectionPtr &conn, json &js, muduo::Timestamp time);

private:
    ChatService();

    // 消息id映射处理函数
    std::unordered_map<int, MsgHandler> _msgHanderMap;

    // 存储连接
    std::unordered_map<int, muduo::net::TcpConnectionPtr> _userConnectionMap;

    // 互斥锁
    std::mutex _connMutex;

    //
    UserModel _usermodel;

    OfflineMsgModel _offlineMsgModel;

    FriendModel _friendModel;

    GroupModel _groupModel;

    Redis _redis;
};