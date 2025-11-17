#include "GroupModel.hpp"
#include "db.hpp"
#include"public.hpp"

bool GroupModel::createGroup(Group &group_)
{
    char sql[200] = {0};
    sprintf(sql, "insert into AllGroup(groupname,groupdesc) values('%s','%s');",
            group_.getName().c_str(), group_.getDesc().c_str());

    MySQL mysql;
    if (mysql.connect())
    {
        if (mysql.update(sql))
        {
            group_.setId(mysql_insert_id(mysql.getConnection()));
            return true;
        }
    }
    return false;
}

void GroupModel::addToGroup(int user_id, int group_id, std::string role)
{
    char sql[200] = {0};
    sprintf(sql, "insert into GroupUser values(%d,%d,'%s');",
            group_id, user_id, role.c_str());

    MySQL mysql;
    if (mysql.connect())
    {
        mysql.update(sql);
    }
}

std::vector<Group> GroupModel::queryGroups(int user_id)
{
    char sql[200] = {0};
    sprintf(sql, "select a.id,a.groupname,a.groupdesc from AllGroup a inner join \ 
        GroupUser b on b.group_id=a.id where b.user_id=%d",
            user_id);

    MySQL mysql;
    std::vector<Group> group_res;

    if (mysql.connect())
    {
        MYSQL_RES *res = mysql.query(sql);
        if (res != nullptr)
        {
            MYSQL_ROW row;
            while ((row = mysql_fetch_row(res)) != nullptr)
            {
                Group group(atoi(row[0]), row[1], row[2]);
                group_res.push_back(group);
            }
            mysql_free_result(res);
        }
    }

    for (Group &group : group_res)
    {
        sprintf(sql, "select a.id,a.name,a.state,b.group_role from User a  \
            inner join GroupUser b on b.user_id=a.id where b.group_id=%d",
                group.getId());

        MYSQL_RES *res = mysql.query(sql);
        if (res != nullptr)
        {
            debug("群组成员列表不为空");
            MYSQL_ROW row;
            while ((row = mysql_fetch_row(res)) != nullptr)
            {
                GroupUser user;
                user.setId(atoi(row[0]));
                user.setName(row[1]);
                user.setState(row[2]);
                user.setRole(row[3]);
                group.getUsers().push_back(user);
            }
            debug_cout("成员数：",group.getUsers().size());
            mysql_free_result(res);
        }
    }
    return group_res;
}

std::vector<int> GroupModel::queryGroupUsers(int user_id, int group_id)
{
    char sql[200] = {0};
    sprintf(sql, "select user_id from GroupUser where group_id=%d and user_id!=%d",
        group_id,user_id);

    MySQL mysql;
    std::vector<int> id_vec;

    if (mysql.connect())
    {
        MYSQL_RES *res = mysql.query(sql);
        if(res!=nullptr)
        {
            MYSQL_ROW row;
            while((row=mysql_fetch_row(res))!=nullptr)
            {
                id_vec.push_back(atoi(row[0]));
            }         
            mysql_free_result(res);
        }
    }
    return id_vec;
}
