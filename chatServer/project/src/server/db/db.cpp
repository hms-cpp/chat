#include "db.hpp"

MySQL::MySQL()
{
    _conn = mysql_init(nullptr);
}

MySQL::~MySQL()
{
    if (_conn != nullptr)
    {
        mysql_close(_conn);
    }
}

bool MySQL::connect()
{
    MYSQL *p = mysql_real_connect(_conn, LOOP_DB_SERVER, USER, PASSWORD, DB_NAME, DB_PORT, nullptr, 0);

    if (p != nullptr)
    {
        mysql_query(_conn, "set names gbk");
        return true;
    }
    return false;
}

bool MySQL::update(std::string sql)
{
    if(mysql_query(_conn,sql.c_str()))
    {
        LOG_INFO<<__FILE__ <<" : "<<__LINE__<<" : "<<sql<<"更新失败";
        return false;
    }
    return true;
}

MYSQL_RES *MySQL::query(std::string sql)
{
    if(mysql_query(_conn,sql.c_str()))
    {
        LOG_INFO<<__FILE__ <<" : "<<__LINE__<<" : "<<sql<<"更新失败";
        return nullptr;
    }
    return mysql_use_result(_conn);
}

MYSQL *MySQL::getConnection()
{
    return _conn;
}
