#ifndef GROUPUSER_HPP
#define FROUPUSER_HPP

#include "user.hpp"

class GroupUser : public User
{
public:
    void setRole(string role) { _role = role; }
    string getRole() { return _role; }
private:
    string _role;
};




#endif