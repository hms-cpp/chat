#pragma once

#include <string>

class User
{
public:
    User(int id = -1, std::string name = "", std::string pwd = "", std::string state = "offline")
    {
        this->id = id;
        this->name = name;
        this->password = pwd;
        this->state = state;
    }

    User(int id,std::string name,std::string state,bool flag)
    {
        this->id=id;
        this->name=name;
        this->state=state;
        this->password="XXX";
    }

    void setId(int id)
    {
        this->id = id;
    }
    void setName(std::string name)
    {
        this->name = name;
    }
    void setPassword(std::string pwd)
    {
        this->password = pwd;
    }
    void setState(std::string state)
    {
        this->state = state;
    }

    int getId()
    {
        return id;
    }
    std::string getName()
    {
        return name;
    }
    std::string getPassword()
    {
        return password;
    }
    std::string getState()
    {
        return state;
    }

private:
    int id;
    std::string name;
    std::string password;
    std::string state;
};