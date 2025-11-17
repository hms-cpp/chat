#pragma once 

#include<string>
#include<list>
#include<vector>

class OfflineMsgModel
{
    public:

    void insert(int user_id,std::string msg);

    void remove(int user_id);

    std::vector<std::string> query(int user_id);

};