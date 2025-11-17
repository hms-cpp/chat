#pragma once 

#include"Group.hpp"

class GroupModel
{
    public:
    //创建群组
    bool createGroup(Group &group_);

    //加入群组
    void addToGroup(int user_id,int group_id,std::string role);

    //查询用户所在群组消息
    std::vector<Group> queryGroups(int user_id);

    //根据指定group_id 查询群组用户id列表
    std::vector<int> queryGroupUsers(int user_id,int group_id);

};