#pragma once

#include<iostream>
#include<string>

//时间戳类
class Timestamp
{
public:
    Timestamp();
    explicit Timestamp(int64_t microSecondsSinceEpoch);//禁止隐式转换构造
    time_t secondsSinceEpoch() const { return static_cast<time_t>(microSecondsSinceEpoch_ / kMicroSecondsPerSecond); }
    static Timestamp now();
    bool valid() const { return microSecondsSinceEpoch_ > 0; }
    std::string toString() const;
    static const int kMicroSecondsPerSecond = 1000 * 1000;
private:
    int64_t microSecondsSinceEpoch_;
};

inline bool operator<(Timestamp lhs, Timestamp rhs)
{
  return lhs.secondsSinceEpoch() < rhs.secondsSinceEpoch();
}

inline Timestamp addTime(Timestamp timestamp, double seconds)
{
    int64_t delta = static_cast<int64_t>(seconds * Timestamp::kMicroSecondsPerSecond);
    return Timestamp(timestamp.secondsSinceEpoch() + delta);
}