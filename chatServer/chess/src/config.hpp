#include"public.hpp"

#define LOOP_IP "127.0.0.1"
#define LOCAL_IP "192.168.121.144"

#define SERVER_PORT 6000
#define NGINX_PORT 8000

#if USE_LOCAL_IP
    #define IP LOCAL_IP
#else
    #define IP LOOP_IP
#endif

#if USE_NGINX_PORT
    #define PORT NGINX_PORT
#else
    #define PORT LOOP_IP
#endif

#define LOOP_DB_SERVER "127.0.0.1"
#define DB_SERVER "192.168.121.144"
#define USER "root"
#define PASSWORD "1539439254@qq.com"
#define DB_NAME "chat"
#define DB_PORT 3306

// 通信数据组成

// 消息类型定义
#define D_msg_type "msg_type"                       // 消息类型
#define D_errno "errno"                             // 错误码
#define D_error_msg "error_msg"                     // 错误信息
#define D_local_user_id "local_user_id"             // 客户端用户id
#define D_local_user_name "local_user_name"         // 客户端用户名
#define D_local_user_password "local_user_password" // 客户端用户密码
#define D_local_user_state "local_user_state"       // 客户端用户状态

// 好友相关字段
#define D_Friends "Friends"                         // 好友列表
#define D_friend_id "friend_id"                     // 好友ID
#define D_friend_name "friend_name"                 // 好友名称
#define D_friend_state "friend_state"               // 好友状态

// 群组相关字段
#define D_Groups "Groups"                           // 群组列表
#define D_group_id "group_id"                       // 群组ID
#define D_group_name "group_name"                   // 群组名称
#define D_group_desc "group_desc"                   // 群组描述

// 群组成员相关字段
#define D_GroupUsers "GroupUsers"                   // 群组成员列表
#define D_groupUser "groupUser"                     // 群组成员
#define D_groupUser_id "groupUser_id"               // 群组成员ID
#define D_groupUser_name "groupUser_name"           // 群组成员名称
#define D_groupUser_state "groupUser_state"         // 群组成员状态
#define D_groupUser_role "groupUser_role"           // 群组成员职位


// 消息相关字段
#define D_message "message"                         // 消息内容
#define D_sendTo_id "sendTo_id"                     // 接收方ID
#define D_offline_message "offline_message"         // 离线消息
/*
客户端关闭数据包
{
    D_msg_type:0
}

客户端用户登录数据包
{
    D_msg_type
    D_local_user_id
    D_local_user_name
}
服务端回复登录消息数据包(登录成功)
{
    D_msg_type
    D_errno
    D_local_user_id
    D_local_user_name
    D_offline_message
    D_Friends{
        D_local_user_id
        D_local_user_name
        D_local_user_state
    }...
    D_Groups{
        D_group_id
        D_group_name
        D_group_desc
        D_GroupUsers{
            D_groupUser_id
            D_groupUser_name
            D_groupUser_state
            D_groupUser_role
        }...
    }...
}
服务端回复登录消息数据包(登录失败)
{
    D_msg_type
    D_errno
    D_error_msg
}
    






*/
