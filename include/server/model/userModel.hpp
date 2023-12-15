#ifndef USERMODEL_HPP
#define USERMODEL_HPP

#include "user.hpp"

// User表的数据操作类
class UserModel
{
public:
    bool insert(User &user);
    // 根据主键id查询用户信息
    User query(int i);
    // 更新用户的状态信息
    bool update(User &user);
    // 重置用户的状态信息
    bool resetState();
};




#endif