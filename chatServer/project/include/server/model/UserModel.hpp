#pragma once 

#include"User.hpp"

class UserModel
{
    public:
    bool insertUser(User &user);

    User query(int id);

    bool update_user_state(User user);

    bool update_user_state_for_client_close(int id);

    void reset_user_state();


    User find_for_march(int id);
};