#pragma once

#include <iostream>
#include <string>
#include<tuple>

#define DEBUG1 1
#define USE_LOCAL_IP 1

#define USE_NGINX_PORT 1


inline void debug(std::string s)
{
    if constexpr (DEBUG1)
    {
        std::cout << s << std::endl;
    }
}

template <typename... Args>
void debug_cout(Args &&...args)
{
    if constexpr (DEBUG1)
    {
        [&]<std::size_t... I>(std::index_sequence<I...>)
        {
            auto tuple = std::forward_as_tuple(std::forward<Args>(args)...);
            ((std::cout << std::get<I * 2>(tuple) << " : " << std::get<I * 2 + 1>(tuple) 
            << std::endl), ...);
        }(std::make_index_sequence<sizeof...(Args) / 2>{});
    }
}

enum EnMsgType
{
    SHUT_DONW = 0,      // 安全关闭连接
    LOGIN_MSG = 1,      // 登录消息
    LOGIN_MSG_ACK = 2,  // 登录消息回复
    REG_MSG = 3,        // 注册消息
    REG_MSG_ACK = 4,    // 注册消息回复
    ONE_CHAT_MSG = 5,   // 一对一聊天消息
    ADD_FRIEND_MSG = 6, // 添加好友

    CREATE_GROUP_MSG = 7, // 创建群组
    ADD_GROUP_MSG = 8,    // 加入群组
    GROUP_CHAT_MSG = 9,   // 群聊天
    
    LOGIN_OUT_MSG=10,     //注销

    MSG_FOR_GAME_CHESS=11,
    MARCH_MSG=12,
};