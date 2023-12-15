
#include "friendModel.hpp"
#include "db.hpp"

// 添加好友关系
bool FriendModel::insert(int userid, int friendid)
{
    // 1.组装sql语句
    char sql[1024] = {0};
    snprintf(sql, sizeof sql, "insert into Friend values ('%d', '%d')",
             userid, friendid);

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
// 返回用户好友列表
vector<User> FriendModel::query(int userid)
{
    // 1.组装sql语句
    char sql[1024] = {0};
    snprintf(sql, sizeof sql, "select a.id,a.name,a.state from User a inner join Friend b on b.friendid = a.id where b.userid=%d",
             userid);
    vector<User> vec;

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
                User user;
                user.setId(atoi(row[0]));
                user.setName(row[1]);
                user.setState(row[2]);
                vec.push_back(user);
            }

            mysql_free_result(res);
        }
    }
    return vec;
}