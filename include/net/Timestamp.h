#ifndef TIME_STAMP_H
#define TIME_STAMP_H
#include <iostream>
#include <string>

class Timestamp
{
public:
    Timestamp();
    explicit Timestamp(int64_t microSecondSinceEpoch);
    ~Timestamp() = default;
    static Timestamp now();
    std::string toString() const;
private:
    int64_t microSecondSinceEpoch_;
};

#endif 