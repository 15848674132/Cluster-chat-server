#include "groupModel.hpp"
#include "db.hpp"
// 创建群组
bool GroupModel::createGroup(Group &group)
{
    // 1.组装sql语句
    char sql[1024] = {0};
    snprintf(sql, sizeof sql, "insert into AllGroup (groupname, groupdesc) values ('%s', '%s')",
             group.getName().c_str(), group.getDesc().c_str());

    // 2.与数据库交互
    MySQL mysql;
    if (mysql.connect())
    {
        if (mysql.update(sql))
        {
            // 获取插入成功的群组数据生成的主键id
            group.setId(mysql_insert_id(mysql.getConnection()));
            return true;
        }
    }

    return false;
}
// 加入群组
bool GroupModel::addGroup(int userid, int groupid, string role)
{
    // 1.组装sql语句
    char sql[1024] = {0};
    snprintf(sql, sizeof sql, "insert into GroupUser (groupid, userid, grouprole) values (%d, %d, '%s')",
             groupid, userid, role.c_str());

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
// 查询用户所在群组信息
vector<Group> GroupModel::queryGroups(int userid)
{
    // 1.组装sql语句
    char sql[1024] = {0};
    snprintf(sql, sizeof sql, "select a.id, a.groupname, a.groupdesc from AllGroup a \
        inner join GroupUser b on a.id = b.groupid where b.userid=%d",
             userid);
    vector<Group> vec;
    // 2.与数据库交互
    MySQL mysql;
    if (mysql.connect())
    {
        MYSQL_RES *res = mysql.query(sql);
        if (res != nullptr)
        {
            MYSQL_ROW row;
            while ((row = mysql_fetch_row(res)) != nullptr)
            {
                Group group;
                group.setId(atoi(row[0]));
                group.setName(row[1]);
                group.setDesc(row[2]);
                vec.push_back(group);
            }
        }
        mysql_free_result(res);
    }

    // 查询群组成员信息
    for (auto &group : vec)
    {
        snprintf(sql, sizeof sql, "select a.id, a.name, a.state, b.grouprole from User a \
            inner join GroupUser b on a.id = b.userid where b.groupid = %d",
                 group.getId());
        MYSQL_RES *res = mysql.query(sql);
        if(res != nullptr)
        {
            MYSQL_ROW row;
            while((row = mysql_fetch_row(res)) != nullptr)
            {
                GroupUser user;
                user.setId(atoi(row[0]));
                user.setName(row[1]);
                user.setState(row[2]);
                user.setRole(row[3]);
                group.getUsers().push_back(user);
            }
            mysql_free_result(res);
        }
    }

    return vec;
}

// 根据指定的groupid查询群组用户id表，除userid自己，主要用户群聊业务给群组其他成员群发消息
vector<int> GroupModel::queryGroupUsers(int userid, int groupid)
{
    // 1.组装sql语句
    char sql[1024] = {0};
    snprintf(sql, sizeof sql, "select userid from GroupUser where groupid = %d and userid != %d",
             groupid, userid);
    vector<int> vec;
    // 2.与数据库交互
    MySQL mysql;
    if (mysql.connect())
    {
        MYSQL_RES *res = mysql.query(sql);
        if (res != nullptr)
        {
            MYSQL_ROW row;
            while ((row = mysql_fetch_row(res)) != nullptr)
            {
                vec.push_back(atoi(row[0]));
            }
        }
    }

    return vec;
}