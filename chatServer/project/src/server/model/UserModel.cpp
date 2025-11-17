#include "UserModel.hpp"
#include "db.hpp"
#include"public.hpp"

#include <iostream>

bool UserModel::insertUser(User &user)
{
    char sql[200] = {0};
    sprintf(sql, "insert into User(name,password,state) values('%s','%s','%s');", user.getName().c_str(), user.getPassword().c_str(), user.getState().c_str());

    MySQL mysql;
    if (mysql.connect())
    {
        if (mysql.update(sql))
        {
            user.setId(mysql_insert_id(mysql.getConnection()));
            return true;
        }
    }
    return false;
}

User UserModel::query(int id)
{
    char sql[200] = {0};
    sprintf(sql, "select * from User where id=%d", id);

    MySQL mysql;
    if (mysql.connect())
    {
        MYSQL_RES *res = mysql.query(sql);
        if(res!=nullptr)
        {
            MYSQL_ROW row=mysql_fetch_row(res);
            if(row!=nullptr)
            {
                User user;
                user.setId(atoi(row[0]));
                user.setName(row[1]);
                user.setPassword(row[2]);
                user.setState(row[3]);
                mysql_free_result(res);
                return user;
            }
        }
    }
    return User();
}

bool UserModel::update_user_state(User user)
{
    char sql[200] = {0};
    sprintf(sql, "update User set state='%s' where id=%d",user.getState().c_str(),user.getId());

    MySQL mysql;
    if (mysql.connect())
    {
        if(mysql.update(sql))
        {
            return true;
        }
    }
    return false;
}

bool UserModel::update_user_state_for_client_close(int id)
{
    char sql[200] = {0};
    sprintf(sql, "update User set state='offline' where id=%d",id);

    MySQL mysql;
    if (mysql.connect())
    {
        if(mysql.update(sql))
        {
            return true;
        }
    }
    return false;
}

void UserModel::reset_user_state()
{
    char sql[50] = "update User set state='offline';";

    MySQL mysql;
    if (mysql.connect())
    {
        mysql.update(sql);
        debug("reset data");
    }
}


User UserModel::find_for_march(int id)
{
    char sql[200] = {0};
    sprintf(sql, "select * from User where id!=%d and state='online'", id);

    MySQL mysql;
    if (mysql.connect())
    {
        MYSQL_RES *res = mysql.query(sql);
        if(res!=nullptr)
        {
            MYSQL_ROW row=mysql_fetch_row(res);
            if(row!=nullptr)
            {
                User user;
                user.setId(atoi(row[0]));
                mysql_free_result(res);
                return user;
            }
        }
    }
    return User();
}