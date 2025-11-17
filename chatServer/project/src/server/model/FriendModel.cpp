#include "FriendModel.hpp"
#include "db.hpp"

void FriendModel::insert(int user_id, int friend_id)
{
    char sql[200] = {0};
    sprintf(sql, "insert into Friend values(%d,%d);", user_id, friend_id);

    MySQL mysql;
    if (mysql.connect())
    {
        mysql.update(sql);
    }
}

std::vector<User> FriendModel::query(int id)
{
    char sql[200] = {0};
    sprintf(sql, "select a.id,a.name,a.state from User a inner join Friend b on b.friend_id=a.id where b.user_id=%d", id);

    MySQL mysql;
    std::vector<User> msg_res;

    if (mysql.connect())
    {
        MYSQL_RES *res = mysql.query(sql);
        if(res!=nullptr)
        {
            MYSQL_ROW row;
            while((row=mysql_fetch_row(res))!=nullptr)
            {
                User user;
                user.setId(atoi(row[0]));
                user.setName(row[1]);
                user.setState(row[2]);
                msg_res.push_back(user);
            }         
            mysql_free_result(res);
        }
    }
    return msg_res;
}
