#include "db.hpp"

// #include <muduo/base/Logging.h>
#include "Logger.h"
// using namespace muduo;

static string server = "101.42.253.57";
static string user = "langkai";
static string password = "123456";
static string dbname = "chat";

MySQL::MySQL()
{
    _conn = mysql_init(nullptr);
}

MySQL::~MySQL()
{
    if(_conn)
        mysql_close(_conn);
}

bool MySQL::connect()
{
    MYSQL *p = mysql_real_connect(_conn, server.c_str(), user.c_str(),
                                password.c_str(), dbname.c_str(), 3306, nullptr, 0);
    if(p != nullptr)
    {
        // mysql_query(_conn, "set names gbk");
        mysql_set_character_set(_conn, "utf8");
        // LOG_INFO << "connect mysql success!";
        LOG_INFO("connect mysql success!");
    }
    else 
    {
        // LOG_INFO << "connect mysql fail!";
        LOG_INFO("connect mysql fail!");

    }

    return p;
}

bool MySQL::update(string sql)
{
    if (mysql_query(_conn, sql.c_str()) != 0)
    {
        // LOG_INFO << __FILE__ << ":" << __LINE__ << ":"
        //          << sql << "更新失败!";
        LOG_INFO("%s:%d: %s更新失败!", __FILE__, __LINE__, sql.c_str());

        return false;
    }
    return true;
}

MYSQL_RES *MySQL::query(string sql)
{
    if (mysql_query(_conn, sql.c_str()))
    {
        // LOG_INFO << __FILE__ << ":" << __LINE__ << ":"
        //          << sql << "查询失败!";
        LOG_INFO("%s:%d: %s查询失败!", __FILE__, __LINE__, sql.c_str());
        return nullptr;
    }

    return mysql_use_result(_conn);
}

MYSQL* MySQL::getConnection()
{
    return _conn;
}