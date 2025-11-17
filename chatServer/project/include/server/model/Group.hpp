#pragma once

#include "GroupUser.hpp"

#include <string>
#include <vector>

class Group
{
public:
    Group()
    {
        
    }
    Group(int g_id, std::string g_name, std::string g_desc)
    {
        this->id = g_id;
        this->name = g_name;
        this->desc = g_desc;
    }
    void setId(int id)
    {
        this->id = id;
    }
    void setName(std::string name)
    {
        this->name = name;
    }
    void setDesc(std::string desc)
    {
        this->desc = desc;
    }
    int getId()
    {
        return id;
    }
    std::string getName()
    {
        return name;
    }
    std::string getDesc()
    {
        return desc;
    }
    std::vector<GroupUser>& getUsers()
    {
        return _users;
    }

private:
    int id;
    std::string name;
    std::string desc;
    std::vector<GroupUser> _users;
};
