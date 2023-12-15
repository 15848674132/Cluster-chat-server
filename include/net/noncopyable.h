
/*
* noncopyable被继承以后，派生类可以正常的构造和析构
* 但是派生类无法进行拷贝和赋值
*/
#ifndef NONCOPYABLE_H
#define NONCOPYABLE_H

class noncopyable
{
public:
    noncopyable(const noncopyable&) = delete;
    noncopyable& operator=(const noncopyable&) = delete;
protected:
    noncopyable() = default;
    ~noncopyable() = default;
};

#endif