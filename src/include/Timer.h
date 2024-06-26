#pragma once

#include "Timestamp.h"
#include "Callbacks.h"
#include "noncopyable.h"

#include<atomic>

class Timer : noncopyable
{
public:
    Timer(TimerCallback cb ,Timestamp when, double interval)
    : timerCallback_(std::move(cb))
    , expiration_(when)
    , interval_(interval)
    , repeat_(interval > 0.0)
    , sequence_(++numCreated_)
    {}

    void run() const
    {
        timerCallback_();
    }

    Timestamp expiration() const { return expiration_; }
    bool repeat() const { return repeat_; }
    int64_t sequence() const { return sequence_;}
    void restart(Timestamp now);

    static int64_t numCreated() { return numCreated_;}
private:
    
    const TimerCallback timerCallback_;
    Timestamp expiration_;
    const double interval_;
    const bool repeat_;
    const int64_t sequence_;
    static std::atomic_int64_t numCreated_;

};