#include "offlineMessageModel.hpp"

#include "db.hpp"
#include <iostream>
using namespace std;

// 存储用户的离线消息
bool OfflineMsgModel::insert(int userid, string msg)
{
    // 1.组装sql语句
    char sql[1024] = {0};
    snprintf(sql, sizeof sql, "insert into OfflineMessage values ('%d', '%s')",
             userid, msg.c_str());

    // 2.与数据库交互
    MySQL mysql;
    if (mysql.connect())
    {
        if (mysql.update(sql))
        {
            return true;
        }
    }
    return false;
}

// 删除用户的离线消息
bool OfflineMsgModel::remove(int userid)
{
    // 1.组装sql语句
    char sql[1024] = {0};
    snprintf(sql, sizeof sql, "delete from OfflineMessage  where userid = %d",
             userid);

    // 2.与数据库交互
    MySQL mysql;
    if (mysql.connect())
    {
        if (mysql.update(sql))
        {
            return true;
        }
    }
    return false;
}

// 查询用户的离线消息
vector<string> OfflineMsgModel::query(int userid)
{
    // 1.组装sql语句
    char sql[1024] = {0};
    snprintf(sql, sizeof sql, "select message from OfflineMessage  where userid = %d",
             userid);
    vector<string> vec;

    // 2.与数据库交互
    MySQL mysql;
    if (mysql.connect())
    {
        MYSQL_RES *res = mysql.query(sql);
        if (res != nullptr)
        {
            MYSQL_ROW row;
            while((row = mysql_fetch_row(res)) != nullptr)
            {
                vec.push_back(row[0]);
            }

            mysql_free_result(res);
            return vec;
        }
    }
    return vec;
}