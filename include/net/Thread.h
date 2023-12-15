#ifndef THREAD_H
#define THREAD_H

#include <functional>
#include <thread>
#include <memory>
#include <string>
#include <atomic>

#include "noncopyable.h"

class Thread : noncopyable
{
public:
    using ThreadFunc = std::function<void()>;

    explicit Thread(ThreadFunc, const std::string &name = std::string());
    ~Thread();

    void start();
    void join();

    bool started() const { return started_; }
    pid_t tid() const { return tid_; }
    const std::string& thName() const { return thName_; }
    static int numCreate() { return numCreate_; }

private:
    void setDefaultName();
private:

    bool started_;
    bool joined_;
    std::shared_ptr<std::thread> thread_;
    pid_t tid_;
    ThreadFunc func_;
    std::string thName_;
    static std::atomic_int32_t numCreate_;
};




#endif