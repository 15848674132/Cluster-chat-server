#ifndef GROUP_HPP
#define GROUP_HPP

#include "groupUser.hpp"
#include <string>
#include <vector>
using namespace std;


class Group
{
public:
    Group(int id = -1, string name = "", string desc = "")
        : _id(id), _name(name), _desc(desc)
    {
    }

    ~Group() = default;

    void setId(const int &id) { _id = id; }
    void setName(const string &name) { _name = name; }
    void setDesc(const string &desc) { _desc = desc; }

    const int& getId() { return _id; }
    const string& getName() { return _name; }
    const string& getDesc() { return _desc; }

    vector<GroupUser> &getUsers() { return _users; }

private:
    int _id;
    string _name;
    string _desc;
    vector<GroupUser> _users;
};





#endif