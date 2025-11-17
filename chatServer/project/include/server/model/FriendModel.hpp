#pragma once 

#include<vector>

#include"User.hpp"

//维护好友信息的操作接口方法
class FriendModel
{
    public:
    //
    void insert(int user_id,int friend_id);

    //返回用户好友列表
    std::vector<User> query(int id);
};