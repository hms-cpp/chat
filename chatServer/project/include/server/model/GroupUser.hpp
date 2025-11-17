#pragma once 

#include"User.hpp"

class GroupUser : public User
{
    public:
    GroupUser()
    {
        this->setId(-1);
        this->setName("null");
        this->setState("null");
        this->_role="null";
    }
    GroupUser(int id,std::string name ,std::string state ,std::string role)
    {
        this->setId(id);
        this->setName(name);
        this->setState(state);
        this->_role=role;
    }
    
    void setRole(std::string role)
    {
        this->_role=role;
    }
    std::string getRole()
    {
        return _role;
    }

    public:
    std::string _role;
};