#ifndef USER_HPP
#define USER_HPP

#include <string>

using namespace std;

// 匹配User表的ORM类
class User
{
public:
    User(int id = -1, string name = "", string password = "", string state = "offline")
        : _id(id), _name(name), _password(password), _state(state)
    {
    }

    ~User() = default;

    void setId(const int &id) { _id = id; }
    void setName(const string &name) { _name = name; }
    void setPassword(const string &password) { _password = password; }
    void setState(const string &state) { _state = state; }

    const int& getId() { return _id; }
    const string& getName() { return _name; }
    const string& getPassword() { return _password; }
    const string& getState() { return _state; }
private:
    int _id;
    string _name;
    string _password;
    string _state;
};

#endif